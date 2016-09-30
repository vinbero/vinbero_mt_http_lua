#ifndef _TUCUBE_HTTP_LUA_H
#define _TUCUBE_HTTP_LUA_H

#include <lua.h>
#include <tucube/tucube_Module.h>
#include <tucube/tucube_ClData.h>

struct tucube_http_lua_TlModule {
   lua_State* L;
};

struct tucube_http_lua_ClData {
    int* clientSocket;
    lua_State* L;
};

int tucube_epoll_http_Module_init(struct tucube_Module_Args* moduleArgs, struct tucube_Module_List* moduleList);
int tucube_epoll_http_Module_tlInit(struct tucube_Module* module, struct tucube_Module_Args* moduleArgs);
int tucube_epoll_http_Module_clInit(struct tucube_Module* module, struct tucube_ClData_List* clDataList, int* clientSocket);

int tucube_epoll_http_Module_onRequestStart(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onRequestMethod(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestUri(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestProtocol(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onGetRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, ssize_t* content_length);

int tucube_epoll_http_Module_onRequestContentType(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestScriptPath(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestHeaderField(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestHeader_Value(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize);
int tucube_epoll_http_Module_onRequestHeadersFinish(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onRequestBodyStart(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onRequestBody(struct tucube_Module* module, struct tucube_ClData* clData, char* bodyChunk, ssize_t bodyChunkSize);
int tucube_epoll_http_Module_onRequestBodyFinish(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onRequestFinish(struct tucube_Module* module, struct tucube_ClData* clData);

int tucube_epoll_http_Module_onResponseStatusCode(struct tucube_Module* module, struct tucube_ClData* clData, int* statusCode);
int tucube_epoll_http_Module_onResponseHeaderStart(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onResponseHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize);
int tucube_epoll_http_Module_onResponseBodyStart(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onResponseBody(struct tucube_Module* module, struct tucube_ClData* clData, const char** body, size_t* bodySize);

int tucube_epoll_http_Module_clDestroy(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_tlDestroy(struct tucube_Module* module);
int tucube_epoll_http_Module_destroy(struct tucube_Module* module);

#endif
