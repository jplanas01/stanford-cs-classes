/**
 * File: request-handler.cc
 * ------------------------
 * Provides the implementation for the HTTPRequestHandler class.
 */

#include "request-handler.h"
#include "response.h"
#include "request.h"
#include "client-socket.h"
#include "ostreamlock.h"
#include <socket++/sockstream.h> // for sockbuf, iosockstream
#include <sstream>
using namespace std;

/* Constructor
 * -----------
 *  Initializes the list of blacklisted domains.
 */
HTTPRequestHandler::HTTPRequestHandler() {
    try {
        blacklist.addToBlacklist(kBlacklistFilename);
    } catch (HTTPProxyException e) {
        cout << oslock << "Creating blacklist failed." << endl << osunlock;
        throw HTTPProxyException(e.what());
    }
}


/* ingestClientRequest
 * -------------------
 *  Attempts to ingest a client's request into req from the given socket stream.
 *  Assumes that ss is initialized and alive. If the request is one that has a
 *  payload (e.g., a GET or POST request), hasPayload is set to true; otherwise,
 *  it is set to false.
 *  client is the IP address of the client that sent the request.
 */
void HTTPRequestHandler::ingestClientRequest(HTTPRequest &req, iosockstream &ss,
                                             const string &client, bool &hasPayload)
throw (HTTPBadRequestException) {
    try {
        req.ingestRequestLine(ss);
    } catch (HTTPBadRequestException e) {
        throw HTTPBadRequestException(e.what());
    }
    req.ingestHeader(ss, client);

    cout << oslock << req.getMethod();
    cout << " " << req.getURL() << endl << osunlock;

    //HEAD has no payload
    if (req.getMethod() == "HEAD") {
        hasPayload = false;
        return;
    }

    req.ingestPayload(ss);
    hasPayload = true;
}

/* addForwardHeaders
 * -----------------
 *  Adds the appropriate forwarding headers to the given request, appending if
 *  necessary or creating the header if necessary.
 *  client is the IP address of the client sending the request.
 */
void HTTPRequestHandler::addForwardHeaders(HTTPRequest &req,
                                           const string &client) {
    req.addHeader("x-forwarded-proto", "http");
    if (req.containsName("x-forwarded-proto")) {
        const string &oldForward = req.headerStringByName("x-forwarded-for");
        string newForward;
        if (oldForward == "") {
            newForward = client;
        } else {
            newForward = oldForward + ", " + client;
        }
        req.removeHeader("x-forwarded-for");
        req.addHeader("x-forwarded-for", newForward);
    }
}

/* serveHTTPError
 * --------------
 *  Serves a response with the specified error code and payload message.
 */
void HTTPRequestHandler::serveHTTPError(iosockstream &ss, int code, string payload) {
    HTTPResponse resp;
    resp.setProtocol("HTTP/1.0");
    resp.setResponseCode(code);
    resp.setPayload(payload);
    
    ss << resp;
    ss.flush();
}

/* sendServerRequest
 * -----------------
 *  Forwards the client's request to the server, adding the appropriate headers
 *  on the way and stores the server's response.
 *  client is the IP address of the client, hasPayload indicates whether the
 *  server's response has a payload should be ingested.
 *
 *  Returns true if the process was successful (especially connection to the
 *  given server), false otherwise.
 */
bool HTTPRequestHandler::sendServerRequest(HTTPRequest &clientReq, 
                                           HTTPResponse &response, const string &client,
                                           bool hasPayload) {
    int sock = createClientSocket(clientReq.getServer(), clientReq.getPort());
    if (sock == kClientSocketError) {
        // Error connecting to server, panic
        return false;
    }
    HTTPRequest newReq = clientReq;

    addForwardHeaders(newReq, client);

    sockbuf outsb(sock);
    iosockstream ssout(&outsb);
    ssout << newReq;
    ssout.flush();

    response.ingestResponseHeader(ssout);
    if (hasPayload) {
        response.ingestPayload(ssout);
    }
    return true;
}

/* gen510Error
 * -----------
 *  Populates the given HTTPResponse with a 510 error and a provided payload.
 */
void HTTPRequestHandler::gen510Error(HTTPResponse &servResponse,
                                     const string message) {
    servResponse.setProtocol("HTTP/1.0");
    servResponse.setResponseCode(510);
    servResponse.setPayload(message);
}

/* hasCircularChain
 * ----------------
 *  Detects if a requests' headers form a circular proxy chain (i.e., this proxy
 *  is trying to append itself to a request that has already passed through this
 *  proxy).
 *
 *  client is the IP address of the client that sent the request.
 *  
 *  Returns true if a chain is detected, false otherwise.
 */
bool HTTPRequestHandler::hasCircularChain(const HTTPRequest &req, const string &client) {
    if (req.containsName("x-forwarded-proto")) {
        string proxies = req.headerStringByName("x-forwarded-for");
        if (proxies.find(client) != string::npos) {
            return true;
        }
    }
    return false;
}

/* serviceProxy
 * ------------
 *  Forwards the client's request to another proxy server, updating forwarding
 *  headers along the way and bouncing back the other proxy's response.
 *
 *  Assumes ss and clientReq have been initialized.
 *
 *  proxy is the address of the proxy receiving the request, port is the port of
 *  the proxy, client is the client's IP address, and hasPayload indicates
 *  whether there's a payload that should be ingested.
 */
void HTTPRequestHandler::serviceProxy(iosockstream &ss, HTTPRequest &clientReq, string proxy,
                  unsigned short port, string client, bool hasPayload) {
    HTTPResponse response;

    int sock = createClientSocket(proxy, port);
    if (sock == kClientSocketError) {
        gen510Error(response, "This proxy is configured to forward to another"
                              " proxy, and that second proxy (" + proxy
                              + ") is refusing connections.");
        ss << response;
        ss.flush();
        return;
    }

    HTTPRequest newReq = clientReq;
    addForwardHeaders(newReq, client);
    newReq.setPath(clientReq.getURL());

    sockbuf outsb(sock);
    iosockstream ssout(&outsb);
    ssout << newReq;
    ssout.flush();

    response.ingestResponseHeader(ssout);
    if (hasPayload) {
        response.ingestPayload(ssout);
    }
    
    ss << response;
    ss.flush();
}

/* serviceRequest
 * -------------
 *  Handles a single HTTP request from a client.
 *
 *  connection.first is the socket file descriptor to the client
 *  connection.second is the IP address of the client
 *  info is a SecondProxyInfo struct that contains information about whether
 *  this request should be bounced to a second proxy or not.
 */
void HTTPRequestHandler::serviceRequest(const pair<int, string>& connection,
        const SecondProxyInfo &info) throw() {
    sockbuf sb(connection.first);
    iosockstream ss(&sb);
    HTTPResponse response;

    HTTPRequest clientRequest;
    bool hasPayload = false;

    try {
        ingestClientRequest(clientRequest, ss, connection.second, hasPayload);
    } catch (HTTPBadRequestException e) {
        cout << oslock << "Bad request from client: " << e.what() << endl << osunlock;
        return;
    }


    if (!blacklist.serverIsAllowed(clientRequest.getServer())) {
        serveHTTPError(ss, 403, "Forbidden Content");
        return;
    }

    HTTPRequest serverRequest;
    HTTPResponse serverResponse;

    if (info.inUse) {
        if (hasCircularChain(clientRequest, connection.second)) {
            serveHTTPError(ss, 503, "Chained proxies may not lead through a cycle.");
            return;
        }
        serviceProxy(ss, clientRequest, info.proxy, info.port, connection.second, hasPayload);
        return;
    }

    webCache.lockRequestMutex(clientRequest);
    bool inCache = webCache.containsCacheEntry(clientRequest, serverResponse);
    if (!inCache) {
        if (!sendServerRequest(clientRequest, serverResponse, connection.second, hasPayload)) {
            gen510Error(serverResponse, "Failed to find an IP address for " + clientRequest.getServer() + ".");
        } else {
            if (webCache.shouldCache(clientRequest, serverResponse)) {
                webCache.cacheEntry(clientRequest, serverResponse);
            }
        }
    }
    webCache.unlockRequestMutex(clientRequest);
    
    ss << serverResponse;
    ss.flush();

}

// Clears the on-disk cache
void HTTPRequestHandler::clearCache() {
    webCache.clear();
}

// Sets the max cache age
void HTTPRequestHandler::setCacheMaxAge(long maxAge) {
    webCache.setMaxAge(maxAge);
}
