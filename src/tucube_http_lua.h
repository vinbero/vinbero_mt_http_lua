#ifndef _TUCUBE_HTTP_LUA_H
#define _TUCUBE_HTTP_LUA_H

#include <lua.h>
#include <tucube/tucube_Module.h>
#include <tucube/tucube_ClData.h>
#include <tucube/tucube_epoll_http_ResponseBody.h>

struct tucube_http_lua_TlModule {
   lua_State* L;
};

struct tucube_http_lua_ClData {
    int* clientSocket;
    lua_State* L;
    luaL_Stream* responseBodyStream;
};

int tucube_epoll_http_Module_init(struct tucube_Module_Config* moduleConfig, struct tucube_Module_List* moduleList);
int tucube_epoll_http_Module_tlInit(struct tucube_Module* module, struct tucube_Module_Config* moduleConfig);
int tucube_epoll_http_Module_clInit(struct tucube_Module* module, struct tucube_ClData_List* clDataList, int* clientSocket);

int tucube_epoll_http_Module_onRequestStart(void* args[]);
int tucube_epoll_http_Module_onRequestMethod(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestUri(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestProtocol(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestContentLength(char* token, ssize_t tokenSize, void* args[]);

int tucube_epoll_http_Module_onRequestContentType(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestScriptPath(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestHeaderField(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestHeaderValue(char* token, ssize_t tokenSize, void* args[]);
int tucube_epoll_http_Module_onRequestHeadersFinish(void* args[]);
int tucube_epoll_http_Module_onRequestBodyStart(void* args[]);
int tucube_epoll_http_Module_onRequestBody(char* bodyChunk, ssize_t bodyChunkSize, void* args[]);
int tucube_epoll_http_Module_onRequestBodyFinish(void* args[]);
int tucube_epoll_http_Module_onRequestFinish(void* args[]);

int tucube_epoll_http_Module_onGetRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, ssize_t* contentLength);
int tucube_epoll_http_Module_onGetRequestIntHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, int* headerValue);
int tucube_epoll_http_Module_onGetRequestDoubleHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, double* headerValue);
int tucube_epoll_http_Module_onGetRequestStringHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, const char** headerValue);

int tucube_epoll_http_Module_onResponseStatusCode(struct tucube_Module* module, struct tucube_ClData* clData, int* statusCode);
int tucube_epoll_http_Module_onResponseHeaderStart(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onResponseHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize);
int tucube_epoll_http_Module_onResponseBodyStart(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_onResponseBody(struct tucube_Module* module, struct tucube_ClData* clData, struct tucube_epoll_http_ResponseBody* responseBody);

int tucube_epoll_http_Module_clDestroy(struct tucube_Module* module, struct tucube_ClData* clData);
int tucube_epoll_http_Module_tlDestroy(struct tucube_Module* module);
int tucube_epoll_http_Module_destroy(struct tucube_Module* module);

#endif
