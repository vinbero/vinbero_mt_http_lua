#include <ctype.h>
#include <err.h>
#include <libgonc/gonc_cast.h>
#include <libgonc/gonc_list.h>
#include <libgonc/gonc_nstrncmp.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tucube/tucube_Module.h>
#include <tucube/tucube_ClData.h>
#include "tucube_http_lua.h"

int tucube_epoll_http_Module_init(struct tucube_Module_Args* moduleArgs, struct tucube_Module_List* moduleList) {
    struct tucube_Module* module = malloc(1 * sizeof(struct tucube_Module));
    GONC_LIST_ELEMENT_INIT(module);
    module->tlModuleKey = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlModuleKey, NULL);
    GONC_LIST_APPEND(moduleList, module);
    return 0;
}

int tucube_epoll_http_Module_tlInit(struct tucube_Module* module, struct tucube_Module_Args* moduleArgs) {
    char* scriptFile = NULL;
    GONC_LIST_FOR_EACH(moduleArgs, struct tucube_Module_Arg, module_arg) {
        if(strncmp("script-file", module_arg->name, sizeof("script-file")) == 0)
	     scriptFile = module_arg->value;
    }
    if(scriptFile == NULL) {
        warnx("%s: %u: Argument script-file is required", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    struct tucube_http_lua_TlModule* tlModule = malloc(sizeof(struct tucube_http_lua_TlModule));
    tlModule->L = luaL_newstate();
    luaL_openlibs(tlModule->L);
    lua_newtable(tlModule->L); // requests
    lua_setglobal(tlModule->L, "requests"); //
    if(luaL_loadfile(tlModule->L, scriptFile) != LUA_OK) { // file
        warnx("%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        pthread_exit(NULL);
    }
    lua_getglobal(tlModule->L, "onInit"); // onInit
    if(lua_isnil(tlModule->L, -1))
        lua_pop(tlModule->L, 1); //
    else {
        if(lua_pcall(tlModule->L, 0, 0, 0) != 0) {
            warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); //errorString
            lua_pop(tlModule->L, 1); //
        }
    }
    pthread_setspecific(*module->tlModuleKey, tlModule);
    return 0;
}

int tucube_epoll_http_Module_clInit(struct tucube_Module* module, struct tucube_ClData_List* clDataList, int* clientSocket) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct tucube_ClData* clData = malloc(sizeof(struct tucube_ClData));
    GONC_LIST_ELEMENT_INIT(clData);
    clData->pointer = malloc(1 * sizeof(struct tucube_http_lua_ClData));
    GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket = clientSocket;
    GONC_LIST_APPEND(clDataList, clData);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *clientSocket); // requests clientSocket
    lua_newtable(tlModule->L); // requests clientSocket request
    lua_pushstring(tlModule->L, "headers"); // requests clientSocket request "headers"
    lua_newtable(tlModule->L); // requests clientSocket request "headers" headers
    lua_settable(tlModule->L, -3); // requests clientSocket request
    lua_pushstring(tlModule->L, "parameters"); // requests clientSocket request "parameters"
    lua_newtable(tlModule->L); // requests clientSocket request "parameters" parameters
    lua_settable(tlModule->L, -3); // requests clientSocket request
    lua_pushstring(tlModule->L, "body"); // requests clientSocket request "body"
    lua_pushstring(tlModule->L, ""); // requests clientSocket request "body" ""
    lua_settable(tlModule->L, -3); // requests clientSocket request
    lua_pushstring(tlModule->L, "contentLength"); // requests clientSocket request "contentLength"
    lua_pushinteger(tlModule->L, 0); // requests clientSocket request "contentLength" 0
    lua_settable(tlModule->L, -3); // requests clientSocket request
    lua_settable(tlModule->L, -3); // requests
    lua_pop(tlModule->L, 1); //
    return 0;
}


int tucube_epoll_http_Module_onRequestStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "onRequestStart"); // onRequestStart
    if(lua_isnil(tlModule->L, -1)) {
        lua_pop(tlModule->L, 1); //
        return 0;
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestStart requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestStart requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestStart requests request
    lua_remove(tlModule->L, -2); // onRequestStart request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1);
        return -1;
    }
    return 0;
}

int tucube_epoll_http_Module_onRequestMethod(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize ) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "method"); // requests request "method"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "method" method 
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 

    return 0;
}

int tucube_epoll_http_Module_onRequestUri(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "requestUri"); // requests request "requestUri"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "requestUri" requestUri 
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); //

    return 0;
}

int tucube_epoll_http_Module_onRequestProtocol(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "protocol"); // requests request "protocol"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "protocol" protocol 
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); //

    return 0;
}

int tucube_epoll_http_Module_onRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "contentLength"); // requests request "contentLength"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "contentLength" contentLength
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

static int tucube_http_lua_onGetRequestContentLength(lua_State* L) {
    // request
    lua_pushstring(L, "contentLength"); // request "contentLength"
    lua_gettable(L, -2); // request contentLength
    return 1;
}

int tucube_epoll_http_Module_onGetRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, ssize_t* contentLength) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "onGetRequestContentLength"); // onGetRequestContentLength
    if(lua_isnil(tlModule->L, -1)) { // nil 
        lua_pop(tlModule->L, 1); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onGetRequestContentLength); // onGetRequestContentLength
    }
    lua_getglobal(tlModule->L, "requests"); // onGetRequestContentLength requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onGetRequestContentLength requests clientSocket
    lua_gettable(tlModule->L, -2); // onGetRequestContentLength requests request
    lua_remove(tlModule->L, -2); // onGetRequestContentLength request
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // requestContentLength
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    *contentLength = lua_tointeger(tlModule->L, -1); // requestContentLength
    lua_pop(tlModule->L, 1); //
    return 0;
}

int tucube_epoll_http_Module_onRequestContentType(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "contentType"); // requests request "contentType"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "contentType" contentType
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

int tucube_epoll_http_Module_onRequestScriptPath(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "scriptPath"); // requests request "scriptPath"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "scriptPath" scriptPath
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

int tucube_epoll_http_Module_onRequestHeaderField(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // requests request "recentHeaderField"
    for(ssize_t index = 0; index < tokenSize; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "recentHeaderField" recentHeaderField
    lua_settable(tlModule->L, -3); //requests request
    lua_pop(tlModule->L, 2); //

    return 0;
}

int tucube_epoll_http_Module_onRequestHeaderValue(struct tucube_Module* module, struct tucube_ClData* clData, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // requests request "recentHeaderField"
    lua_gettable(tlModule->L, -2); // requests request recentHeaderField
    lua_pushvalue(tlModule->L, -2); // requests request recentHeaderField request
    lua_pushstring(tlModule->L, "headers"); // requests request recentHeaderField request "headers"
    lua_gettable(tlModule->L, -2); // requests request recentHeaderField request headers
    lua_pushvalue(tlModule->L, -3); // requests request recentHeaderField request headers requestHeaderField
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request recentHeaderField request headers recentHeaderField token
    lua_settable(tlModule->L, -3); // requests request recentHeaderField request headers
    lua_pop(tlModule->L, 3); // requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // requests request "recentHeaderField"
    lua_pushnil(tlModule->L); // requests request "recentHeaderField" nil
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); //
    return 0;
}

int tucube_epoll_http_Module_onRequestHeadersFinish(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "onRequestHeadersFinish"); // onRequestHeadersFinish
    if(lua_isnil(tlModule->L, -1)) {
        lua_pop(tlModule->L, 1); //
        return 0;
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestHeadersFinish requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestHeadersFinish requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestHeadersFinish requests request
    lua_remove(tlModule->L, -2); // onRequestHeadersFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    return 0;
}

int tucube_http_lua_onRequestBodyStart(lua_State* L) {
    // request
    lua_pushstring(L, "body"); // request "body"
    lua_newtable(L); // request "body" body
    lua_settable(L, -3); // request
    return 0;
}

int tucube_epoll_http_Module_onRequestBodyStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "onRequestBodyStart"); // onRequestBodyStart
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); // 
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyStart); // onRequestBodyStart
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestBodyStart requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestBodyStart requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestBodyStart requests request
    lua_remove(tlModule->L, -2); // onRequestBodyStart request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
    }
}

int tucube_http_lua_onRequestBody(lua_State* L) {
    // request bodyChunk
    lua_getglobal(L, "table"); //  request bodyChunk table
    lua_pushstring(L, "insert"); // request bodyChunk table "insert"
    lua_gettable(L, -2); // request bodyChunk table insert
    lua_pushvalue(L, 1); // request bodyChunk table insert request
    lua_pushstring(L, "body"); // request bodyChunk table insert request "body"
    lua_gettable(L, -2); // request bodyChunk table insert request body
    lua_remove(L, -2); // request bodyChunk table insert body
    lua_pushvalue(L, 2); // request bodyChunk table insert body bodyChunk
    lua_pcall(L, 2, 0, 0); // request bodyChunk table
    lua_pop(L, 1); // request bodyChunk
    return 0;
}

int tucube_epoll_http_Module_onRequestBody(struct tucube_Module* module, struct tucube_ClData* clData, char* bodyChunk, ssize_t bodyChunkSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "onRequestBody"); // onRequestBody
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); // 
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBody); // onRequestBody
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestBody requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestBody requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestBody requests request
    lua_remove(tlModule->L, -2); // onRequestBody request
    lua_pushlstring(tlModule->L, bodyChunk, bodyChunkSize); // onRequestBody request bodyChunk
    if(lua_pcall(tlModule->L, 2, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    return 0;
}

int tucube_http_lua_onRequestBodyFinish(lua_State* L) {
    // request
    lua_pushstring(L, "body"); // request "body"
    lua_gettable(L, -2); // request body
    lua_getglobal(L, "table"); // request body table
    lua_pushstring(L, "concat"); // request body table "concat"
    lua_gettable(L, -2); // request body table concat
    lua_pushvalue(L, -3); // request body table concat body
    if(lua_pcall(L, 1, 1, 0) != 0) { // request body table result
        warnx("%s: %u, %s", __FILE__, __LINE__, lua_tostring(L, -1)); // errorString
        lua_pop(L, 1); //
    }
    lua_pushvalue(L, -4); // request body table result request
    lua_pushstring(L, "body"); // request body table result request "body"
    lua_pushvalue(L, -3); // request body table result request "body" result
    lua_settable(L, -3); // request body table result request
    lua_pop(L, 4); // request body
    return 0;
}

int tucube_epoll_http_Module_onRequestBodyFinish(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "onRequestBodyFinish"); // onRequestBodyFinish
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyFinish); // onRequestBodyFinish
    } 
    lua_getglobal(tlModule->L, "requests"); // onRequestBodyFinish requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestBodyFinish requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestBodyFinish requests request
    lua_remove(tlModule->L, -2); // onRequestBodyFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    return 0;
}

int tucube_epoll_http_Module_onRequestFinish(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    const char* requestUri;
    const char* script_path;
    size_t scriptPathSize;
    const char* pathInfo;
    const char* queryString;
    lua_pushstring(tlModule->L, "requestUri"); // requests request "requestUri"
    lua_gettable(tlModule->L, -2); //requests request requestUri
    requestUri = lua_tostring(tlModule->L, -1); // requests request requestUri
    lua_pop(tlModule->L, 1); // requests request
    lua_pushstring(tlModule->L, "scriptPath"); // requests request "scriptPath"
    lua_gettable(tlModule->L, -2); // requests request scriptPath
    if(lua_isnil(tlModule->L, -1)) { // requests request scriptPath 
        lua_pop(tlModule->L, 1); // requests request
	    lua_pushstring(tlModule->L, "scriptPath"); // requests request "scriptPath"
        script_path = "";
        scriptPathSize = sizeof("") - 1;
        lua_pushstring(tlModule->L, ""); // requests request "scriptPath" "" 
        lua_settable(tlModule->L, -3); // requests request
    }
    else {
        script_path = lua_tolstring(tlModule->L, -1, &scriptPathSize); // requests request scriptPath 
        if(strncmp(script_path + scriptPathSize - 1, "/", sizeof("/") - 1) == 0)
            return -1;
        lua_pop(tlModule->L, 1); // requests request
    }
    if((pathInfo = strstr(requestUri, script_path)) != requestUri) // if request uri doesn't begin with script name
        return -1;
    pathInfo += scriptPathSize;
    if((queryString = strstr(pathInfo, "?")) != NULL) { // check if there is query string
        ++queryString; // query string begins after the question mark
        if(strstr(queryString, "?") != NULL) // check if there is unnecessary question mark after query string
            return -1;

        lua_pushstring(tlModule->L, "pathInfo"); // requests request "pathInfo"

        if(queryString - pathInfo - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlModule->L, pathInfo, queryString - pathInfo - 1); // requests request headers "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // requests request
        lua_pushstring(tlModule->L, "queryString"); // requests request "queryString"
        lua_pushstring(tlModule->L, queryString); // requests request "queryString" queryString 
        lua_settable(tlModule->L, -3); // requests request 
    }
    else {
        lua_pushstring(tlModule->L, "pathInfo"); // requests request "pathInfo"
        if(strlen(pathInfo) != 0) // check if path info is not empty string
            lua_pushstring(tlModule->L, pathInfo); // requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // requests request
    }
    lua_getglobal(tlModule->L, "onRequestFinish"); // requests request onRequestFinish
    if(lua_isnil(tlModule->L, -1)) {
        warnx("%s: %u: onRequestFinish() is not found in the script", __FILE__, __LINE__);
        return -1;
    }
    lua_pushvalue(tlModule->L, -2); // requests request onRequestFinish request
    if(lua_pcall(tlModule->L, 1, 3, 0) != 0) { // requests request statusCode headers body
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // requests request errorString
        lua_pop(tlModule->L, 4); //
        return -1;
    }
    lua_remove(tlModule->L, 1); // request statusCode headers body
    lua_remove(tlModule->L, 1); // statusCode headers body
    if(!(lua_isnumber(tlModule->L, -3) && lua_istable(tlModule->L, -2) && (lua_isstring(tlModule->L, -1) || lua_isfunction(tlModule->L, -1) || lua_isnil(tlModule->L, -1)))) {
        lua_pop(tlModule->L, lua_gettop(tlModule->L)); //
        lua_pushinteger(tlModule->L, 500); // 500
        lua_newtable(tlModule->L); // 500 headers
        lua_pushstring(tlModule->L, "Content-Type"); // 500 headers "Content-Type"
        lua_pushstring(tlModule->L, "text/plain; charset=utf8"); // 500 headers "Content-Type" "text/plain; charset=utf8"
        lua_settable(tlModule->L, -3); // 500 headers 
        lua_pushstring(tlModule->L, "500 Internal Server Error"); // 500 headers "Internal swerver error"
    }
    return 0;
}

int tucube_epoll_http_Module_onResponseStatusCode(struct tucube_Module* module, struct tucube_ClData* clData, int* statusCode) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    *statusCode = lua_tointeger(tlModule->L, -3); // statusCode headers body
    return 0;
}

int tucube_epoll_http_Module_onResponseHeaderStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_pushnil(tlModule->L); // statusCode headers body nil
    if(lua_next(tlModule->L, -3) != 0) // statusCode headers body new_headerField new_headerValue
        return 1;
    return 0;
}

int tucube_epoll_http_Module_onResponseHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    *headerField = lua_tolstring(tlModule->L, -2, headerFieldSize); // statusCode headers body headerField headerValue
    *headerValue = lua_tolstring(tlModule->L, -1, headerValueSize); // statusCode headers body headerField headerValue
    lua_pop(tlModule->L, 1); // statusCode headers body headerField
    if(lua_next(tlModule->L, -3) != 0) // statusCode headers body headerField ->  statusCode headers body (new_headerField new_headerValue || empty)
        return 1;
    return 0;
}

int tucube_epoll_http_Module_onResponseBodyStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    if(lua_isfunction(tlModule->L, -1)) {
        GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L = lua_newthread(tlModule->L); // statusCode headers body thread
        return 1;
    }
    else if(lua_isstring(tlModule->L, -1)) {
        GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L = NULL;
        return 1;
    }
    else if(lua_isnil(tlModule->L, -1))
        return 0;
    return -1;
}

int tucube_epoll_http_Module_onResponseBody(struct tucube_Module* module, struct tucube_ClData* clData, const char** body, size_t* bodySize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    if(lua_isfunction(tlModule->L, -2)) { // statusCode headers body thread
        if(lua_gettop(GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L) > 1)
            lua_pop(GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, 1);
        int coroutine_result;
        lua_pushvalue(tlModule->L, -2); // statusCode headers body thread body
        lua_xmove(tlModule->L, GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, 1); // statusCode headers body thread
        coroutine_result = lua_resume(GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, tlModule->L, 0); // statusCode headers body thread
        *body = lua_tolstring(GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, -1, bodySize); // statusCode headers body thread
        if(coroutine_result == LUA_YIELD)
            return 1; // statusCode headers body thread
        lua_pop(tlModule->L, 1); // statusCode headers body
        if(coroutine_result == 0)
            return 0;
        return -1;
    }
    else if(lua_isstring(tlModule->L, -1)) { // statusCode headers body
        *body = lua_tolstring(tlModule->L, -1, bodySize);
        return 0;
    }
    return -1;
}

int tucube_epoll_http_Module_clDestroy(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_pop(tlModule->L, lua_gettop(tlModule->L)); //
    close(*GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket);
    *GONC_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket = -1;
    free(clData->pointer);
    free(clData);
    return 0;
}

int tucube_epoll_http_Module_tlDestroy(struct tucube_Module* module) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    if(tlModule != NULL) {
        lua_getglobal(tlModule->L, "onDestroy"); // onDestroy
        if(lua_isnil(tlModule->L, -1))
            lua_pop(tlModule->L, 1); //
        else {
            if(lua_pcall(tlModule->L, 0, 0, 0) != 0) {
                warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); //errorString
                lua_pop(tlModule->L, 1); //
            }
        }
        lua_close(tlModule->L);
        free(tlModule);
    }
    return 0;
}

int tucube_epoll_http_Module_destroy(struct tucube_Module* module) {
    pthread_key_delete(*module->tlModuleKey);
    free(module->tlModuleKey);
    free(module);
    return 0;
}
