/**
 * File: news-aggregator.cc
 * ------------------------
 * Presents the implementation of the NewsAggregator class.
 */

#include "news-aggregator.h"
#include "log.h"
#include "liberal-news-aggregator.h"
#include "conservative-news-aggregator.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>
#include <thread>

#include <getopt.h>
#include <libxml/parser.h>
#include <libxml/catalog.h>
#include "rss-feed-list.h"
#include "rss-feed.h"
#include "utils.h"
#include "string-utils.h"

using namespace std;

static const string kDefaultRSSFeedListURL = "small-feed.xml";
NewsAggregator *NewsAggregator::createNewsAggregator(int argc, char *argv[]) {
  struct option options[] = {
    {"verbose", no_argument, NULL, 'v'},
    {"quiet", no_argument, NULL, 'q'},
    {"url", required_argument, NULL, 'u'},
    {"conserve-threads", no_argument, NULL, 'c'},
    {NULL, 0, NULL, 0},
  };

  string rssFeedListURI = kDefaultRSSFeedListURL;
  bool verbose = false;
  bool conserve = false;
  while (true) {
    int ch = getopt_long(argc, argv, "vqcu:", options, NULL);
    if (ch == -1) break;
    switch (ch) {
    case 'v':
      verbose = true;
      break;
    case 'q':
      verbose = false;
      break;
    case 'u':
      rssFeedListURI = optarg;
      break;
    case 'c':
      conserve = true;
      break;
    default:
      NewsAggregatorLog::printUsage("Unrecognized flag.", argv[0]);
    }
  }
  
  argc -= optind;
  if (argc > 0) NewsAggregatorLog::printUsage("Too many arguments.", argv[0]);
  
  if (conserve)
    return new ConservativeNewsAggregator(rssFeedListURI, verbose);
  else
    return new LiberalNewsAggregator(rssFeedListURI, verbose);
}

NewsAggregator::NewsAggregator(const string& rssFeedListURI, bool verbose):
  rssFeedListURI(rssFeedListURI), log(verbose), built(false),
    feedSem(maxRssFeedThreads),
    articleSem(maxArticleThreads) {}

void NewsAggregator::buildIndex() {
  if (built) return;
  built = false;
  xmlInitParser();
  xmlInitializeCatalog();
  processAllFeeds();
  xmlCatalogCleanup();
  xmlCleanupParser();
}

static const size_t kMaxMatchesToShow = 15;
void NewsAggregator::queryIndex() const {
  while (true) {
    cout << "Enter a search term [or just hit <enter> to quit]: ";
    string response;
    getline(cin, response);
    response = trim(response);
    if (response.empty()) break;
    const vector<pair<Article, int> >& matches = index.getMatchingArticles(response);
    if (matches.empty()) {
      cout << "Ah, we didn't find the term \"" << response << "\". Try again." << endl;
    } else {
      cout << "That term appears in " << matches.size() << " article" 
           << (matches.size() == 1 ? "" : "s") << ".  ";
      if (matches.size() > kMaxMatchesToShow) 
        cout << "Here are the top " << kMaxMatchesToShow << " of them:" << endl;
      else if (matches.size() > 1)
        cout << "Here they are:" << endl;
      else
        cout << "Here it is:" << endl;
      size_t count = 0;
      for (const pair<Article, int>& match: matches) {
        if (count == kMaxMatchesToShow) break;
        count++;
        string title = match.first.title;
        if (shouldTruncate(title)) title = truncate(title);
        string url = match.first.url;
        if (shouldTruncate(url)) url = truncate(url);
        string times = match.second == 1 ? "time" : "times";
        cout << "  " << setw(2) << setfill(' ') << count << ".) "
             << "\"" << title << "\" [appears " << match.second << " " << times << "]." << endl;
        cout << "       \"" << url << "\"" << endl;
      }
    }
  }
}

/* processDuplicate
 * ----------------
 *
 *  Processes duplicate articles as given in the assignment spec. A running
 *  intersection of tokens is kept for all duplicate articles and the shortest
 *  URL is kept track of. Eventually, these are added to the index as the final
 *  token list and article URL.
 */
void NewsAggregator::processDuplicate(map<string, TokenContainer> &dups, TokenContainer &cont) {
    // uses article title and server as key, if there's a match there's a
    // duplicate.

    TokenContainer orig = dups[cont.art.title + getURLServer(cont.art.url)];
    TokenContainer newCont = {};
    set_intersection(orig.tokens.cbegin(), orig.tokens.cend(),
                     cont.tokens.cbegin(), cont.tokens.cend(),
                     back_inserter(newCont.tokens));
    newCont.art = orig.art < cont.art ? orig.art : cont.art;
    dups[cont.art.title + getURLServer(cont.art.url)] = newCont;
}

/* Returns true if there has been an attempt to download the given URL before.
 * If the URL hasn't been downloaded, it's marked as attempted (done here to
 * prevent having to relock the mutex and add additional overhead).
 */
bool NewsAggregator::isDuplicateURL(string &url) {
    seenMutex.lock();
    unordered_set<string>::iterator seenIter;
    seenIter = seenURL.find(url);
    if (seenIter == seenURL.end()) {
        seenMutex.unlock();
        return false;
    }
    seenURL.insert(url);
    seenMutex.unlock();

    return true;
}

/* fetchArticle
 * ------------
 *  
 *  Fetches a single article within a feed and adds it to the index, skipping it
 *  if it is found to be a duplicate article. The number of simultaneous
 *  connections to the same server is limited to 6 at any time.
 *
 *  dups is a map that is used to process duplicates, art is the article that
 *  will be downloaded and added to the index.
 */
void NewsAggregator::fetchArticle(map<string, TokenContainer> &dups, Article art) {
    HTMLDocument html(art.url);
    try {
        html.parse();
    } catch (HTMLDocumentException e) {
        cout << oslock << "Error in fetching article: " << e.what() << endl << osunlock;
        return;
    }

    TokenContainer cont = {art, html.getTokens()};
    sort(cont.tokens.begin(), cont.tokens.end());

    dupsMutex.lock();
    map<string, TokenContainer>::iterator dupIter;
    dupIter = dups.find(art.title + getURLServer(art.url));
    if (dupIter != dups.end()) {
        processDuplicate(dups, cont);
    } else {
        dups[art.title + getURLServer(cont.art.url)] = cont;
    }
    dupsMutex.unlock();
    articleSem.signal(on_thread_exit);
}
