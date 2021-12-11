/**
 * File: request-handler.h
 * -----------------------
 * Defines the HTTPRequestHandler class, which fully proxies and
 * services a single client request.  
 */

#ifndef _request_handler_
#define _request_handler_

#include <utility>
#include <string>
#include <socket++/sockstream.h>
#include "request.h"
#include "blacklist.h"
#include "cache.h"
#include "second-proxy.h"

const std::string kBlacklistFilename = "blocked-domains.txt";

class HTTPRequestHandler {
 public:
  HTTPRequestHandler();
  void serviceRequest(const std::pair<int, std::string>& connection,
          const struct SecondProxyInfo &info) throw();
  void clearCache();
  void setCacheMaxAge(long maxAge);
 private:
  void ingestClientRequest(HTTPRequest &req, 
                           iosockstream &ss,
                           const std::string& client, bool& hasPayload) 
      throw (HTTPBadRequestException);
  void genRequestToServer(HTTPRequest &req, HTTPRequest &clientRequest, 
                          const std::string &client);
  void addForwardHeaders(HTTPRequest &req, const std::string &client);
  void serveHTTPError(iosockstream &ss, int code, std::string payload);
  bool sendServerRequest(HTTPRequest &clientReq,HTTPResponse &response, 
                         const std::string &client, bool hasPayload);
  void gen510Error(HTTPResponse &response, std::string message);
  bool hasCircularChain(const HTTPRequest &request, const std::string &client);
  void serviceProxy(iosockstream &ss, HTTPRequest &clientRequest, std::string proxy,
                  unsigned short port, std::string client, bool hasPayload);

  HTTPBlacklist blacklist;
  HTTPCache webCache;
};

#endif
