/**
 * File: scheduler.cc
 * ------------------
 * Presents the implementation of the HTTPProxyScheduler class.
 */

#include "scheduler.h"
#include <utility>
using namespace std;

void HTTPProxyScheduler::scheduleRequest(int clientfd, const string& clientIPAddress,
        const struct SecondProxyInfo &info) throw () {
    requestThreads.schedule([this, clientfd, clientIPAddress, &info] {
        requestHandler.serviceRequest(make_pair(clientfd, clientIPAddress), info);
        });
}
