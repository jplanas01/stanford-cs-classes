/**
 * File: liberal-news-aggregator.h
 * -------------------------------
 * Defines the subclass of NewsAggregator that is openly liberal about its
 * use of threads.  While it is smart enough to limit the number of threads
 * that can exist at any one time, it does not try to converve threads
 * by pooling or reusing them.  Instead, it creates a new thread
 * every time something needs to be downloaded.  It's easy, but wasteful.
 */

#pragma once
#include "news-aggregator.h"

class LiberalNewsAggregator : public NewsAggregator {
public:
  LiberalNewsAggregator(const std::string& rssFeedListURI, bool verbose);

protected:  
  // implement the one abstract method required of all concrete subclasses
  void processAllFeeds();

private:
  void processSingleFeed(std::map<std::string, TokenContainer> &dups, std::string url);

  std::map<std::string, std::unique_ptr<semaphore> > serverConns;
};
