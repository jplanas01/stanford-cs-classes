/**
 * File: conservative-news-aggregator.cc
 * -------------------------------------
 * Presents the implementation of the ConservativeNewsAggregator class.
 */

#include "conservative-news-aggregator.h"
#include "rss-feed-list.h"
#include "rss-feed.h"
#include <iostream>
using namespace std;

ConservativeNewsAggregator::ConservativeNewsAggregator(const string& rssFeedListURI, bool verbose) : 
  NewsAggregator(rssFeedListURI, verbose) {}


/* processSingleFeed
 * -----------------
 *
 *  Adds the articles from a single RSS feed to the index.
 *  dups is a map that is used to handle potential duplicated articles, url is
 *  the URL of the feed to process. The number of article downloading threads is
 *  limited to 24.
 */
void ConservativeNewsAggregator::processSingleFeed(map<string, TokenContainer> &dups, string url) {
    if (isDuplicateURL(url)) {
        return;
    }

    RSSFeed feed(url);
    try {
        feed.parse();
    } catch (RSSFeedException e) {
        cout << oslock << "Error in fetching RSS Feed: " << e.what() << endl << osunlock;
        return;
    }

    ThreadPool articlePool(maxArticleThreads);
    for (auto const &art : feed.getArticles()) {
        articlePool.schedule([this, &dups, &art] { fetchArticle(dups, art); });
    }
    articlePool.wait();
}

void ConservativeNewsAggregator::processAllFeeds() {
    RSSFeedList rssList(rssFeedListURI);
    try {
        rssList.parse();
    } catch (RSSFeedListException e) {
        cout << oslock << "Parsing the RSS feed list faield: " << e.what() << endl << osunlock;
        return;
    }

    map<string, TokenContainer> dups;
    
    ThreadPool feedThreads(maxRssFeedThreads);

    const map<string, string> &urlTitles = rssList.getFeeds();
    for (auto const &iterator : urlTitles) {
        // Schedule single feed
        feedThreads.schedule([this, &dups, iterator] {processSingleFeed(dups, iterator.first);});
    }
    feedThreads.wait();

    // Add all articles from aux. data structure to index
    for (auto const &tok : dups) {
        index.add(tok.second.art, tok.second.tokens);
    }
}
