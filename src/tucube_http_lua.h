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

int tucube_epoll_http_Module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list);
int tucube_epoll_http_Module_threadLocalInit(struct tucube_module* module, struct tucube_module_args* module_args);
int tucube_epoll_http_Module_clientLocalInit(struct tucube_module* module, struct tucube_cldata_list* cldata_list, int* cilent_socket);

int tucube_epoll_http_Module_onRequestStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_onRequestMethod(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestUri(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestProtocol(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestContentLength(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onGetRequestContentLength(struct tucube_module* module, struct tucube_cldata* cldata, ssize_t* content_length);

int tucube_epoll_http_Module_onRequestContentType(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestScriptPath(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestHeaderField(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestHeader_Value(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestHeadersFinish(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_onRequestBodyStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_onRequestBody(struct tucube_module* module, struct tucube_cldata* cldata, char* bodyChunk, ssize_t bodyChunkSize);
int tucube_epoll_http_Module_onRequestBodyFinish(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_onRequestFinish(struct tucube_module* module, struct tucube_cldata* cldata);

int tucube_epoll_http_Module_onResponseStatusCode(struct tucube_module* module, struct tucube_cldata* cldata, int* status_code);
int tucube_epoll_http_Module_onResponseHeaderStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_onResponseHeader(struct tucube_module* module, struct tucube_cldata* cldata, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize);
int tucube_epoll_http_Module_onResponseBodyStart(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_onResponseBody(struct tucube_module* module, struct tucube_cldata* cldata, const char** body, size_t* bodySize);

int tucube_epoll_http_Module_clientLocalDestroy(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_Module_threadLocalDestroy(struct tucube_module* module);
int tucube_epoll_http_Module_destroy(struct tucube_module* module);

#endif
