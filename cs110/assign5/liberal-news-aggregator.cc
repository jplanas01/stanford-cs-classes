/**
 * File: liberal-news-aggregator.cc
 * --------------------------------
 * Presents the implementation of the LiberalNewsAggregator class.
 */

#include "liberal-news-aggregator.h"
#include "rss-feed-list.h"
#include "rss-feed.h"
#include "utils.h"
#include "ostreamlock.h"
#include <iostream>
#include <algorithm>
#include <thread>
using namespace std;

LiberalNewsAggregator::LiberalNewsAggregator(const string& rssFeedListURI, bool verbose) : 
    NewsAggregator(rssFeedListURI, verbose) { }


/* processSingleFeed
 * -----------------
 *
 *  Adds the articles from a single RSS feed to the index.
 *  dups is a map that is used to handle potential duplicated articles, url is
 *  the URL of the feed to process. The number of article downloading threads is
 *  limited to 24
 */
void LiberalNewsAggregator::processSingleFeed(map<string, TokenContainer> &dups, string url) {
    if (isDuplicateURL(url)) {
        return;
    }

    vector<thread> articleThreads;

    RSSFeed feed(url);
    try {
        feed.parse();
    } catch (RSSFeedException e) {
        cout << oslock << "Error in fetching RSS Feed: " << e.what() << endl << osunlock;
        feedSem.signal(on_thread_exit);
        return;
    }

    for (auto const &art : feed.getArticles()) {
        articleSem.wait();
        string server = getURLServer(url);

        connsMutex.lock();
        unique_ptr<semaphore> &conn = serverConns[server];
        if (conn == nullptr) {
            conn.reset(new semaphore(6));
        }
        connsMutex.unlock();
        conn->wait();

        articleThreads.push_back(thread([this, &dups, &art, &conn]{fetchArticle(dups, art); conn->signal(on_thread_exit);}));
    }
    feedSem.signal(on_thread_exit);
    for (thread &t : articleThreads) {
        t.join();
    }
}

/* processAllFeeds
 * ---------------
 *
 *  Adds all RSS feed articles in the URI that was passed into the constructor
 *  of the class to the index.
 */
void LiberalNewsAggregator::processAllFeeds() {
    RSSFeedList rssList(rssFeedListURI);
    try {
        rssList.parse();
    } catch (RSSFeedListException e) {
        cout << oslock << "Parsing the RSS feed list faield: " << e.what() << endl << osunlock;
        return;
    }

    map<string, TokenContainer> dups;
    vector<thread> feedThreads;
    const map<string, string> &urlTitles = rssList.getFeeds();
    for (auto const &iterator : urlTitles) {
        feedSem.wait();
        feedThreads.push_back(thread([this, &dups, iterator]{processSingleFeed(dups, iterator.first);}));
    }

    for (thread &t : feedThreads) {
        t.join();
    }

    // Add all articles from aux. data structure to index
    for (auto const &tok : dups) {
        index.add(tok.second.art, tok.second.tokens);
    }

    // Free all memory from allocated semaphores
    for (auto &ptr : serverConns) {
        ptr.second.release();
    }
}
