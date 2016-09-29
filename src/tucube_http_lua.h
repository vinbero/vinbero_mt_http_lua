#ifndef _TUCUBE_HTTP_LUA_H
#define _TUCUBE_HTTP_LUA_H

#include <lua.h>
#include <tucube/tucube_module.h>
#include <tucube/tucube_cldata.h>

struct tucube_http_lua_ThreadLocalModule {
   lua_State* L;
};

struct tucube_http_lua_ClientLocalData {
    int* clientSocket;
    lua_State* L;
};

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list);
int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args);
int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_cldata_list* cldata_list, int* cilent_socket);

int tucube_epoll_http_module_onRequestStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_onRequestMethod(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestUri(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestProtocol(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestContentLength(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onGetRequestContentLength(struct tucube_module* module, struct tucube_cldata* cldata, ssize_t* content_length);

int tucube_epoll_http_module_onRequestContentType(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestScriptPath(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestHeaderField(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestHeader_Value(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_module_onRequestHeadersFinish(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_onRequestBodyStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_onRequestBody(struct tucube_module* module, struct tucube_cldata* cldata, char* bodyChunk, ssize_t bodyChunkSize);
int tucube_epoll_http_module_onRequestBodyFinish(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_onRequestFinish(struct tucube_module* module, struct tucube_cldata* cldata);

int tucube_epoll_http_module_onResponseStatusCode(struct tucube_module* module, struct tucube_cldata* cldata, int* status_code);
int tucube_epoll_http_module_onResponseHeaderStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_onResponseHeader(struct tucube_module* module, struct tucube_cldata* cldata, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize);
int tucube_epoll_http_module_onResponseBodyStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_onResponseBody(struct tucube_module* module, struct tucube_cldata* cldata, const char** body, size_t* bodySize);

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_tldestroy(struct tucube_module* module);
int tucube_epoll_http_module_destroy(struct tucube_module* module);

#endif
