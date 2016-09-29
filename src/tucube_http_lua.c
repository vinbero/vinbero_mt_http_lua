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
#include <tucube/tucube_module.h>
#include <tucube/tucube_cldata.h>
#include "tucube_http_lua.h"

int tucube_epoll_http_module_init(struct tucube_module_args* moduleArgs, struct tucube_module_list* moduleList) {
    struct tucube_module* module = malloc(1 * sizeof(struct tucube_module));
    GONC_LIST_ELEMENT_INIT(module);
    module->tlmodule_key = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlmodule_key, NULL);
    GONC_LIST_APPEND(moduleList, module);
    return 0;
}

int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* moduleArgs) {
    char* scriptFile = NULL;
    GONC_LIST_FOR_EACH(moduleArgs, struct tucube_module_arg, module_arg) {
        if(strncmp("script-file", module_arg->name, sizeof("script-file")) == 0)
	     scriptFile = module_arg->value;
    }
    if(scriptFile == NULL) {
        warnx("%s: %u: Argument script-file is required", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    struct tucube_http_lua_ThreadLocalModule* tlmodule = malloc(sizeof(struct tucube_http_lua_ThreadLocalModule));
    tlmodule->L = luaL_newstate();
    luaL_openlibs(tlmodule->L);
    lua_newtable(tlmodule->L); // requests
    lua_setglobal(tlmodule->L, "requests"); //
    if(luaL_loadfile(tlmodule->L, scriptFile) != LUA_OK) { // file
        warnx("%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    if(lua_pcall(tlmodule->L, 0, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1); //
        pthread_exit(NULL);
    }
    lua_getglobal(tlmodule->L, "onInit"); // onInit
    if(lua_isnil(tlmodule->L, -1))
        lua_pop(tlmodule->L, 1); //
    else {
        if(lua_pcall(tlmodule->L, 0, 0, 0) != 0) {
            warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); //errorString
            lua_pop(tlmodule->L, 1); //
        }
    }
    pthread_setspecific(*module->tlmodule_key, tlmodule);
    return 0;
}

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_cldata_list* cldata_list, int* clientSocket) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    struct tucube_cldata* cldata = malloc(sizeof(struct tucube_cldata));
    GONC_LIST_ELEMENT_INIT(cldata);
    cldata->pointer = malloc(1 * sizeof(struct tucube_http_lua_ClientLocalData));
    GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket = clientSocket;
    GONC_LIST_APPEND(cldata_list, cldata);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *clientSocket); // requests clientSocket
    lua_newtable(tlmodule->L); // requests clientSocket request
    lua_pushstring(tlmodule->L, "headers"); // requests clientSocket request "headers"
    lua_newtable(tlmodule->L); // requests clientSocket request "headers" headers
    lua_settable(tlmodule->L, -3); // requests clientSocket request
    lua_pushstring(tlmodule->L, "parameters"); // requests clientSocket request "parameters"
    lua_newtable(tlmodule->L); // requests clientSocket request "parameters" parameters
    lua_settable(tlmodule->L, -3); // requests clientSocket request
    lua_pushstring(tlmodule->L, "body"); // requests clientSocket request "body"
    lua_pushstring(tlmodule->L, ""); // requests clientSocket request "body" ""
    lua_settable(tlmodule->L, -3); // requests clientSocket request
    lua_pushstring(tlmodule->L, "contentLength"); // requests clientSocket request "contentLength"
    lua_pushinteger(tlmodule->L, 0); // requests clientSocket request "contentLength" 0
    lua_settable(tlmodule->L, -3); // requests clientSocket request
    lua_settable(tlmodule->L, -3); // requests
    lua_pop(tlmodule->L, 1); //
    return 0;
}


int tucube_epoll_http_module_onRequestStart(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onRequestStart"); // onRequestStart
    if(lua_isnil(tlmodule->L, -1)) {
        lua_pop(tlmodule->L, 1); //
        return 0;
    }
    lua_getglobal(tlmodule->L, "requests"); // onRequestStart requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // onRequestStart requests clientSocket
    lua_gettable(tlmodule->L, -2); // onRequestStart requests request
    lua_remove(tlmodule->L, -2); // onRequestStart request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1);
        return -1;
    }
    return 0;
}

int tucube_epoll_http_module_onRequestMethod(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize ) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "method"); // requests request "method"
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "method" method 
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 

    return 0;
}

int tucube_epoll_http_module_onRequestUri(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "requestURI"); // requests request "requestURI"
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "requestURI" requestURI 
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_onRequestProtocol(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "protocol"); // requests request "protocol"
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "protocol" protocol 
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_onRequestContentLength(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); //requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "contentLength"); // requests request "contentLength"
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "contentLength" contentLength
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 
    return 0;
}

static int tucube_http_lua_onGetRequestContentLength(lua_State* L) {
    // request
    lua_pushstring(L, "contentLength"); // request "contentLength"
    lua_gettable(L, -2); // request contentLength
    return 1;
}

int tucube_epoll_http_module_onGetRequestContentLength(struct tucube_module* module, struct tucube_cldata* cldata, ssize_t* contentLength) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onGetRequestContentLength"); // onGetRequestContentLength
    if(lua_isnil(tlmodule->L, -1)) { // nil 
        lua_pop(tlmodule->L, 1); //
        lua_pushcfunction(tlmodule->L, tucube_http_lua_onGetRequestContentLength); // onGetRequestContentLength
    }
    lua_getglobal(tlmodule->L, "requests"); // onGetRequestContentLength requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // onGetRequestContentLength requests clientSocket
    lua_gettable(tlmodule->L, -2); // onGetRequestContentLength requests request
    lua_remove(tlmodule->L, -2); // onGetRequestContentLength request
    if(lua_pcall(tlmodule->L, 1, 1, 0) != 0) { // requestContentLength
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1); //
        return -1;
    }
    *contentLength = lua_tointeger(tlmodule->L, -1); // requestContentLength
    lua_pop(tlmodule->L, 1); //
    return 0;
}

int tucube_epoll_http_module_onRequestContentType(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); //requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "contentType"); // requests request "contentType"
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "contentType" contentType
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 
    return 0;
}

int tucube_epoll_http_module_onRequestScriptPath(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); //requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "scriptPath"); // requests request "scriptPath"
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "scriptPath" scriptPath
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 
    return 0;
}

int tucube_epoll_http_module_onRequestHeaderField(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); //requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "recentHeaderField"); // requests request "recentHeaderField"
    for(ssize_t index = 0; index < tokenSize; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request "recentHeaderField" recentHeaderField
    lua_settable(tlmodule->L, -3); //requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_onRequestHeaderValue(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t tokenSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "recentHeaderField"); // requests request "recentHeaderField"
    lua_gettable(tlmodule->L, -2); // requests request recentHeaderField
    lua_pushvalue(tlmodule->L, -2); // requests request recentHeaderField request
    lua_pushstring(tlmodule->L, "headers"); // requests request recentHeaderField request "headers"
    lua_gettable(tlmodule->L, -2); // requests request recentHeaderField request headers
    lua_pushvalue(tlmodule->L, -3); // requests request recentHeaderField request headers requestHeaderField
    lua_pushlstring(tlmodule->L, token, tokenSize); // requests request recentHeaderField request headers recentHeaderField token
    lua_settable(tlmodule->L, -3); // requests request recentHeaderField request headers
    lua_pop(tlmodule->L, 3); // requests request
    lua_pushstring(tlmodule->L, "recentHeaderField"); // requests request "recentHeaderField"
    lua_pushnil(tlmodule->L); // requests request "recentHeaderField" nil
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); //
    return 0;
}

int tucube_epoll_http_module_onRequestHeadersFinish(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onRequestHeadersFinish"); // onRequestHeadersFinish
    if(lua_isnil(tlmodule->L, -1)) {
        lua_pop(tlmodule->L, 1); //
        return 0;
    }
    lua_getglobal(tlmodule->L, "requests"); // onRequestHeadersFinish requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // onRequestHeadersFinish requests clientSocket
    lua_gettable(tlmodule->L, -2); // onRequestHeadersFinish requests request
    lua_remove(tlmodule->L, -2); // onRequestHeadersFinish request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1); //
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

int tucube_epoll_http_module_onRequestBodyStart(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onRequestBodyStart"); // onRequestBodyStart
    if(lua_isnil(tlmodule->L, -1)) { // nil
        lua_pop(tlmodule->L, 1); // 
        lua_pushcfunction(tlmodule->L, tucube_http_lua_onRequestBodyStart); // onRequestBodyStart
    }
    lua_getglobal(tlmodule->L, "requests"); // onRequestBodyStart requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // onRequestBodyStart requests clientSocket
    lua_gettable(tlmodule->L, -2); // onRequestBodyStart requests request
    lua_remove(tlmodule->L, -2); // onRequestBodyStart request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1); //
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

int tucube_epoll_http_module_onRequestBody(struct tucube_module* module, struct tucube_cldata* cldata, char* bodyChunk, ssize_t bodyChunkSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onRequestBody"); // onRequestBody
    if(lua_isnil(tlmodule->L, -1)) { // nil
        lua_pop(tlmodule->L, 1); // 
        lua_pushcfunction(tlmodule->L, tucube_http_lua_onRequestBody); // onRequestBody
    }
    lua_getglobal(tlmodule->L, "requests"); // onRequestBody requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // onRequestBody requests clientSocket
    lua_gettable(tlmodule->L, -2); // onRequestBody requests request
    lua_remove(tlmodule->L, -2); // onRequestBody request
    lua_pushlstring(tlmodule->L, bodyChunk, bodyChunkSize); // onRequestBody request bodyChunk
    if(lua_pcall(tlmodule->L, 2, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1); //
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

int tucube_epoll_http_module_onRequestBodyFinish(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onRequestBodyFinish"); // onRequestBodyFinish
    if(lua_isnil(tlmodule->L, -1)) { // nil
        lua_pop(tlmodule->L, 1); //
        lua_pushcfunction(tlmodule->L, tucube_http_lua_onRequestBodyFinish); // onRequestBodyFinish
    } 
    lua_getglobal(tlmodule->L, "requests"); // onRequestBodyFinish requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // onRequestBodyFinish requests clientSocket
    lua_gettable(tlmodule->L, -2); // onRequestBodyFinish requests request
    lua_remove(tlmodule->L, -2); // onRequestBodyFinish request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // errorString
        lua_pop(tlmodule->L, 1); //
        return -1;
    }
    return 0;
}

int tucube_epoll_http_module_onRequestFinish(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket); // requests clientSocket
    lua_gettable(tlmodule->L, -2); // requests request
    const char* request_uri;
    const char* script_path;
    size_t script_path_size;
    const char* path_info;
    const char* query_string;
    lua_pushstring(tlmodule->L, "requestURI"); // requests request "requestURI"
    lua_gettable(tlmodule->L, -2); //requests request requestURI
    request_uri = lua_tostring(tlmodule->L, -1); // requests request requestURI
    lua_pop(tlmodule->L, 1); // requests request
    lua_pushstring(tlmodule->L, "scriptPath"); // requests request "scriptPath"
    lua_gettable(tlmodule->L, -2); // requests request scriptPath
    if(lua_isnil(tlmodule->L, -1)) { // requests request scriptPath 
        lua_pop(tlmodule->L, 1); // requests request
	    lua_pushstring(tlmodule->L, "scriptPath"); // requests request "scriptPath"
        script_path = "";
        script_path_size = sizeof("") - 1;
        lua_pushstring(tlmodule->L, ""); // requests request "scriptPath" "" 
        lua_settable(tlmodule->L, -3); // requests request
    }
    else {
        script_path = lua_tolstring(tlmodule->L, -1, &script_path_size); // requests request scriptPath 
        if(strncmp(script_path + script_path_size - 1, "/", sizeof("/") - 1) == 0)
            return -1;
        lua_pop(tlmodule->L, 1); // requests request
    }
    if((path_info = strstr(request_uri, script_path)) != request_uri) // if request uri doesn't begin with script name
        return -1;
    path_info += script_path_size;
    if((query_string = strstr(path_info, "?")) != NULL) { // check if there is query string
        ++query_string; // query string begins after the question mark
        if(strstr(query_string, "?") != NULL) // check if there is unnecessary question mark after query string
            return -1;

        lua_pushstring(tlmodule->L, "pathInfo"); // requests request "pathInfo"

        if(query_string - path_info - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlmodule->L, path_info, query_string - path_info - 1); // requests request headers "pathInfo" pathInfo 
        else
            lua_pushstring(tlmodule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlmodule->L, -3); // requests request
        lua_pushstring(tlmodule->L, "queryString"); // requests request "queryString"
        lua_pushstring(tlmodule->L, query_string); // requests request "queryString" queryString 
        lua_settable(tlmodule->L, -3); // requests request 
    }
    else {
        lua_pushstring(tlmodule->L, "pathInfo"); // requests request "pathInfo"
        if(strlen(path_info) != 0) // check if path info is not empty string
            lua_pushstring(tlmodule->L, path_info); // requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlmodule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlmodule->L, -3); // requests request
    }
    lua_getglobal(tlmodule->L, "onRequestFinish"); // requests request onRequestFinish
    if(lua_isnil(tlmodule->L, -1)) {
        warnx("%s: %u: onRequestFinish() is not found in the script", __FILE__, __LINE__);
        return -1;
    }
    lua_pushvalue(tlmodule->L, -2); // requests request onRequestFinish request
    if(lua_pcall(tlmodule->L, 1, 3, 0) != 0) { // requests request statusCode headers body
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // requests request errorString
        lua_pop(tlmodule->L, 4); //
        return -1;
    }
    lua_remove(tlmodule->L, 1); // request statusCode headers body
    lua_remove(tlmodule->L, 1); // statusCode headers body
    if(!(lua_isnumber(tlmodule->L, -3) && lua_istable(tlmodule->L, -2) && (lua_isstring(tlmodule->L, -1) || lua_isfunction(tlmodule->L, -1) || lua_isnil(tlmodule->L, -1)))) {
        lua_pop(tlmodule->L, lua_gettop(tlmodule->L)); //
        lua_pushinteger(tlmodule->L, 500); // 500
        lua_newtable(tlmodule->L); // 500 headers
        lua_pushstring(tlmodule->L, "Content-Type"); // 500 headers "Content-Type"
        lua_pushstring(tlmodule->L, "text/plain; charset=utf8"); // 500 headers "Content-Type" "text/plain; charset=utf8"
        lua_settable(tlmodule->L, -3); // 500 headers 
        lua_pushstring(tlmodule->L, "500 Internal Server Error"); // 500 headers "Internal swerver error"
    }
    return 0;
}

int tucube_epoll_http_module_onResponseStatusCode(struct tucube_module* module, struct tucube_cldata* cldata, int* statusCode) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *statusCode = lua_tointeger(tlmodule->L, -3); // statusCode headers body
    return 0;
}

int tucube_epoll_http_module_onResponseHeaderStart(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pushnil(tlmodule->L); // statusCode headers body nil
    if(lua_next(tlmodule->L, -3) != 0) // statusCode headers body new_headerField new_headerValue
        return 1;
    return 0;
}

int tucube_epoll_http_module_onResponseHeader(struct tucube_module* module, struct tucube_cldata* cldata, const char** headerField, size_t* headerField_size, const char** headerValue, size_t* headerValueSize) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *headerField = lua_tolstring(tlmodule->L, -2, headerField_size); // statusCode headers body headerField headerValue
    *headerValue = lua_tolstring(tlmodule->L, -1, headerValueSize); // statusCode headers body headerField headerValue
    lua_pop(tlmodule->L, 1); // statusCode headers body headerField
    if(lua_next(tlmodule->L, -3) != 0) // statusCode headers body headerField ->  statusCode headers body (new_headerField new_headerValue || empty)
        return 1;
    return 0;
}

int tucube_epoll_http_module_onResponseBodyStart(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(lua_isfunction(tlmodule->L, -1)) {
        GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L = lua_newthread(tlmodule->L); // statusCode headers body thread
        return 1;
    }
    else if(lua_isstring(tlmodule->L, -1)) {
        GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L = NULL;
        return 1;
    }
    else if(lua_isnil(tlmodule->L, -1))
        return 0;
    return -1;
}

int tucube_epoll_http_module_onResponseBody(struct tucube_module* module, struct tucube_cldata* cldata, const char** body, size_t* body_size) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(lua_isfunction(tlmodule->L, -2)) { // statusCode headers body thread
        if(lua_gettop(GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L) > 1)
            lua_pop(GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L, 1);
        int coroutine_result;
        lua_pushvalue(tlmodule->L, -2); // statusCode headers body thread body
        lua_xmove(tlmodule->L, GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L, 1); // statusCode headers body thread
        coroutine_result = lua_resume(GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L, tlmodule->L, 0); // statusCode headers body thread
        *body = lua_tolstring(GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->L, -1, body_size); // statusCode headers body thread
        if(coroutine_result == LUA_YIELD)
            return 1; // statusCode headers body thread
        lua_pop(tlmodule->L, 1); // statusCode headers body
        if(coroutine_result == 0)
            return 0;
        return -1;
    }
    else if(lua_isstring(tlmodule->L, -1)) { // statusCode headers body
        *body = lua_tolstring(tlmodule->L, -1, body_size);
        return 0;
    }
    return -1;
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_cldata* cldata) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pop(tlmodule->L, lua_gettop(tlmodule->L)); //
    close(*GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket);
    *GONC_CAST(cldata->pointer, struct tucube_http_lua_ClientLocalData*)->clientSocket = -1;
    free(cldata->pointer);
    free(cldata);
    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module) {
    struct tucube_http_lua_ThreadLocalModule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(tlmodule != NULL) {
        lua_getglobal(tlmodule->L, "onDestroy"); // onDestroy
        if(lua_isnil(tlmodule->L, -1))
            lua_pop(tlmodule->L, 1); //
        else {
            if(lua_pcall(tlmodule->L, 0, 0, 0) != 0) {
                warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); //errorString
                lua_pop(tlmodule->L, 1); //
            }
        }
        lua_close(tlmodule->L);
        free(tlmodule);
    }
    return 0;
}

int tucube_epoll_http_module_destroy(struct tucube_module* module) {
    pthread_key_delete(*module->tlmodule_key);
    free(module->tlmodule_key);
    free(module);
    return 0;
}
