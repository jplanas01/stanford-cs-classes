/**
 * File: news-aggregator.h
 * -----------------------
 * Defines the NewsAggregator class, which understands how to 
 * build an index of all HTML articles reachable from a 
 * command-line-provided RSS feed list URI and interact
 * with a user interested in searching that index.
 *
 * Note that NewsAggregator is an abstract base class, and it
 * exists to be subclassed and unify state and logic common to
 * all subclasses into one location.
 */

#pragma once
#include <string>

#include "log.h"
#include "rss-index.h"
#include "html-document.h"
#include <map>                                                                                                                                                                                       
#include <unordered_set>                                                                                                                                                                             
#include <mutex>                                                                                                                                                                                     
#include <memory>                                                                                                                                                                                    
#include <map>
                                                                                                                                                                                                     
#include "semaphore.h"                                                                                                                                                                               
#include "article.h"                                                                                                                                                                                 
#include "rss-feed.h"
#include "ostreamlock.h"

struct TokenContainer {                                                                                                                                                                              
    Article art;                                                                                                                                                                                     
    std::vector<std::string> tokens;                                                                                                                                                                 
};
const int maxRssFeedThreads = 6;                                                                                                                                                                     
const int maxArticleThreads = 24;

class NewsAggregator {
 public: // define those entries that everyone can see and touch
  static NewsAggregator *createNewsAggregator(int argc, char *argv[]);
  virtual ~NewsAggregator() {}
  
  void buildIndex();
  void queryIndex() const;
  
 protected: // defines those entries that only subclasses can see and touch
  std::string rssFeedListURI;
  NewsAggregatorLog log;
  RSSIndex index;
  bool built;
  std::unordered_set<std::string> seenURL;
  std::mutex dupsMutex;                                                                                                                                                                              
  std::mutex seenMutex;                                                                                                                                                                              
  std::mutex connsMutex;                                                                                                                                                                             
  semaphore feedSem;                                                                                                                                                                                 
  semaphore articleSem;
  
  void processDuplicate(std::map<std::string, TokenContainer> &conts, TokenContainer &cont);                                                                                                         
  bool isDuplicateURL(std::string &url); 
  void fetchArticle(std::map<std::string, TokenContainer> &dups, Article art);

  NewsAggregator(const std::string& rssFeedListURI, bool verbose);
  virtual void processAllFeeds() = 0;
  
 private: // defines those entries that only NewsAggregator methods (not even subclasses) can see and touch
  NewsAggregator(const NewsAggregator& original) = delete;
  NewsAggregator& operator=(const NewsAggregator& rhs) = delete;

};
