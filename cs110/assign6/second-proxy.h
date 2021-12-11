#ifndef _second_proxy_h
#define _second_proxy_h
/* Struct that holds information about the a second proxy, if there is any.
 */
struct SecondProxyInfo {
    bool inUse;
    std::string proxy;
    unsigned short port;
};
#endif

