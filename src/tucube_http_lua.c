#include <ctype.h>
#include <err.h>
#include <libgon_c/gon_c_cast.h>
#include <libgon_c/gon_c_list.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <tucube/tucube_Module.h>
#include <tucube/tucube_ClData.h>
#include <tucube/tucube_epoll_http_ResponseBody.h>
#include "tucube_http_lua.h"

int tucube_epoll_http_Module_init(struct tucube_Module_Config* moduleConfig, struct tucube_Module_List* moduleList) {
    struct tucube_Module* module = malloc(1 * sizeof(struct tucube_Module));
    GON_C_LIST_ELEMENT_INIT(module);
    module->tlModuleKey = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlModuleKey, NULL);
    GON_C_LIST_APPEND(moduleList, module);
    json_t* scriptArg = json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArg");
    if(scriptArg != NULL && json_is_object(scriptArg)) {
        json_object_set_new(
            json_array_get(moduleConfig->json, 1),
            "tucube_http_lua.scriptArg",
            json_string(json_dumps(scriptArg, 0))
        );
    }
    return 0;
}

int tucube_epoll_http_Module_tlInit(struct tucube_Module* module, struct tucube_Module_Config* moduleConfig) {
    const char* scriptFile = NULL;
    if(json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptFile") != NULL) {
        scriptFile = json_string_value(
            json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptFile")
        );
    }
    if(scriptFile == NULL) {
        warnx("%s: %u: Argument tucube_http_lua.scriptFile is required", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    struct tucube_http_lua_TlModule* tlModule = malloc(sizeof(struct tucube_http_lua_TlModule));
    tlModule->L = luaL_newstate();
    luaL_openlibs(tlModule->L);
    if(json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArg") != NULL) {
        lua_pushstring(
            tlModule->L,
            json_string_value(
                json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArg")
            )
        ); // arg
    } else
        lua_pushnil(tlModule->L); // nil
    lua_setglobal(tlModule->L, "arg"); //
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
        if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { //
            warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
            lua_pop(tlModule->L, 1); //
        }
    }
    pthread_setspecific(*module->tlModuleKey, tlModule);
    return 0;
}

int tucube_epoll_http_Module_clInit(struct tucube_Module* module, struct tucube_ClData_List* clDataList, int* clientSocket) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct tucube_ClData* clData = malloc(sizeof(struct tucube_ClData));
    GON_C_LIST_ELEMENT_INIT(clData);
    clData->pointer = malloc(1 * sizeof(struct tucube_http_lua_ClData));
    GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket = clientSocket;
    GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->responseBodyStream = NULL;
    GON_C_LIST_APPEND(clDataList, clData);

warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
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


int tucube_epoll_http_Module_onRequestStart(void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_pop(tlModule->L, lua_gettop(tlModule->L)); //
    lua_getglobal(tlModule->L, "onRequestStart"); // onRequestStart
    if(lua_isnil(tlModule->L, -1)) {
        lua_pop(tlModule->L, 1); //
        return 0;
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestStart requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestStart requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestStart requests request
    lua_remove(tlModule->L, -2); // onRequestStart request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1);
        return -1;
    }
    return 0;
}

int tucube_epoll_http_Module_onRequestMethod(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);

warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "method"); // requests request "method"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "method" method 
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

int tucube_epoll_http_Module_onRequestUri(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "requestUri"); // requests request "requestUri"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "requestUri" requestUri 
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); //
    return 0;
}

int tucube_epoll_http_Module_onRequestProtocol(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "protocol"); // requests request "protocol"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "protocol" protocol 
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); //
    return 0;
}

int tucube_epoll_http_Module_onRequestContentLength(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "contentLength"); // requests request "contentLength"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "contentLength" contentLength
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

int tucube_epoll_http_Module_onRequestContentType(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "contentType"); // requests request "contentType"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "contentType" contentType
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

int tucube_epoll_http_Module_onRequestScriptPath(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "scriptPath"); // requests request "scriptPath"
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "scriptPath" scriptPath
    lua_settable(tlModule->L, -3); // requests request
    lua_pop(tlModule->L, 2); // 
    return 0;
}

int tucube_epoll_http_Module_onRequestHeaderField(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); //requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // requests request "recentHeaderField"
    for(ssize_t index = 0; index < tokenSize; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlModule->L, token, tokenSize); // requests request "recentHeaderField" recentHeaderField
    lua_settable(tlModule->L, -3); //requests request
    lua_pop(tlModule->L, 2); //
    return 0;
}

int tucube_epoll_http_Module_onRequestHeaderValue(char* token, ssize_t tokenSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
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

int tucube_epoll_http_Module_onRequestHeadersFinish(void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "onRequestHeadersFinish"); // onRequestHeadersFinish
    if(lua_isnil(tlModule->L, -1)) {
        lua_pop(tlModule->L, 1); //
        return 0;
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestHeadersFinish requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestHeadersFinish requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestHeadersFinish requests request
    lua_remove(tlModule->L, -2); // onRequestHeadersFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    return 0;
}

static int tucube_http_lua_onRequestBodyStart(lua_State* L) {
    // request
    lua_pushstring(L, "body"); // request "body"
    lua_newtable(L); // request "body" body
    lua_settable(L, -3); // request
    return 0;
}

int tucube_epoll_http_Module_onRequestBodyStart(void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
    lua_getglobal(tlModule->L, "onRequestBodyStart"); // onRequestBodyStart
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); // 
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyStart); // onRequestBodyStart
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestBodyStart requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestBodyStart requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestBodyStart requests request
    lua_remove(tlModule->L, -2); // onRequestBodyStart request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
    }
}

static int tucube_http_lua_onRequestBody(lua_State* L) {
    // request bodyChunk
    lua_getglobal(L, "table"); // request bodyChunk table
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

int tucube_epoll_http_Module_onRequestBody(char* bodyChunk, ssize_t bodyChunkSize, void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
    lua_getglobal(tlModule->L, "onRequestBody"); // onRequestBody
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); // 
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBody); // onRequestBody
    }
    lua_getglobal(tlModule->L, "requests"); // onRequestBody requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestBody requests clientSocket
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

static int tucube_http_lua_onRequestBodyFinish(lua_State* L) {
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
    lua_pop(L, 4); // request
    return 0;
}

int tucube_epoll_http_Module_onRequestBodyFinish(void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
    lua_getglobal(tlModule->L, "onRequestBodyFinish"); // onRequestBodyFinish
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyFinish); // onRequestBodyFinish
    } 
    lua_getglobal(tlModule->L, "requests"); // onRequestBodyFinish requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onRequestBodyFinish requests clientSocket
    lua_gettable(tlModule->L, -2); // onRequestBodyFinish requests request
    lua_remove(tlModule->L, -2); // onRequestBodyFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { //
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    return 0;
}

int tucube_epoll_http_Module_onRequestFinish(void* args[]) {
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
    lua_getglobal(tlModule->L, "requests"); // requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // requests request
    const char* requestUri;
    const char* scriptPath;
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
        scriptPath = "";
        scriptPathSize = sizeof("") - 1;
        lua_pushstring(tlModule->L, ""); // requests request "scriptPath" "" 
        lua_settable(tlModule->L, -3); // requests request
    }
    else {
        scriptPath = lua_tolstring(tlModule->L, -1, &scriptPathSize); // requests request scriptPath 
        if(strncmp(scriptPath + scriptPathSize - 1, "/", sizeof("/") - 1) == 0) { // requests request scriptPath
            warnx("%s: %u: scriptPath should not  end with /", __FILE__, __LINE__);
            lua_pop(tlModule->L, 3); //
            return -1;
        }
        lua_pop(tlModule->L, 1); // requests request
    }
    if((pathInfo = strstr(requestUri, scriptPath)) != requestUri) { // requests request scriptPath // if request uri doesn't begin with script name
        warnx("%s: %u: request uri doesn't begin with script name", __FILE__, __LINE__);
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    pathInfo += scriptPathSize;
    if((queryString = strstr(pathInfo, "?")) != NULL) { // check if there is query string
        ++queryString; // query string begins after the question mark
        if(strstr(queryString, "?") != NULL) { // check if there is unnecessary question mark after query string
            warnx("%s: %u: Unnecessary question mark after query string", __FILE__, __LINE__);
            lua_pop(tlModule->L, 2); //
            return -1;
        }
 
        lua_pushstring(tlModule->L, "pathInfo"); // requests request "pathInfo"
        if(queryString - pathInfo - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlModule->L, pathInfo, queryString - pathInfo - 1); // requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // requests request
        lua_pushstring(tlModule->L, "queryString"); // requests request "queryString"
        lua_pushstring(tlModule->L, queryString); // requests request "queryString" queryString 
        lua_settable(tlModule->L, -3); // requests request 
    }
    else {
        lua_pushstring(tlModule->L, "queryString"); // requests request "queryString"
        lua_pushstring(tlModule->L, ""); // requests request "queryString" ""
        lua_settable(tlModule->L, -3); // requests request
        lua_pushstring(tlModule->L, "pathInfo"); // requests request "pathInfo"
        if(strlen(pathInfo) != 0) // check if path info is not empty string
            lua_pushstring(tlModule->L, pathInfo); // requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // requests request
    }
    lua_getglobal(tlModule->L, "onRequestFinish"); // requests request onRequestFinish
    if(lua_isnil(tlModule->L, -1)) { // requests request nil
        warnx("%s: %u: onRequestFinish() is not found in the script", __FILE__, __LINE__);
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    lua_pushvalue(tlModule->L, -2); // requests request onRequestFinish request
    if(lua_pcall(tlModule->L, 1, 3, 0) != 0) { // requests request statusCode headers body
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // requests request errorString
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    lua_remove(tlModule->L, 1); // request statusCode headers body
    lua_remove(tlModule->L, 1); // statusCode headers body
    if(!(lua_isnumber(tlModule->L, -3) && lua_istable(tlModule->L, -2) && (lua_isstring(tlModule->L, -1) || lua_isfunction(tlModule->L, -1) || lua_isnil(tlModule->L, -1) || lua_isuserdata(tlModule->L, -1)))) {
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

int tucube_epoll_http_Module_onGetRequestIntHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, int* headerValue) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "requests"); // * requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // * requests request
    lua_pushstring(tlModule->L, "headers"); // * requests request "headers"
    lua_gettable(tlModule->L, -2); // * requests request headers
    lua_getglobal(tlModule->L, "string"); // * requests request headers string
    lua_pushstring(tlModule->L, "upper"); // * requests request headers string "upper"
    lua_gettable(tlModule->L, -2); // * requests request headers string upper
    lua_pushstring(tlModule->L, headerField); // * requests request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * requests request headers string upperedHeaderField
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * requests request headers string errorString
        lua_pop(tlModule->L, 5); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * requests request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * requests request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * requests request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 4); // *
        return -1;
    }
    *headerValue = lua_tointeger(tlModule->L, -1); // * requests request headers headerValue;
    lua_pop(tlModule->L, 4); // *
    return 0;
}

int tucube_epoll_http_Module_onGetRequestDoubleHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, double* headerValue) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // * requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // * requests request
    lua_pushstring(tlModule->L, "headers"); // * requests request "headers"
    lua_gettable(tlModule->L, -2); // * requests request headers
    lua_getglobal(tlModule->L, "string"); // * requests request headers string
    lua_pushstring(tlModule->L, "upper"); // * requests request headers string "upper"
    lua_gettable(tlModule->L, -2); // * requests request headers string upper
    lua_pushstring(tlModule->L, headerField); // * requests request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * requests request headers string upperedHeaderField
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * requests request headers string errorString
        lua_pop(tlModule->L, 5); //
        return -1;
    }
    lua_remove(tlModule->L, -2); // * requests request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * requests request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * requests request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 4);
        return -1;
    }
    *headerValue = lua_tonumber(tlModule->L, -1); // * requests request headers headerValue;
    lua_pop(tlModule->L, 4); // * 
    return 0;
}

int tucube_epoll_http_Module_onGetRequestStringHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, const char** headerValue) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "requests"); // * requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // requests clientSocket
    lua_gettable(tlModule->L, -2); // * requests request
    lua_pushstring(tlModule->L, "headers"); // * requests request "headers"
    lua_gettable(tlModule->L, -2); // * requests request headers
    lua_getglobal(tlModule->L, "string"); // * requests request headers string
    lua_pushstring(tlModule->L, "upper"); // * requests request headers string "upper"
    lua_gettable(tlModule->L, -2); // * requests request headers string upper
    lua_pushstring(tlModule->L, headerField); // * requests request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * requests request headers string upperedHeaderField
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * requests request headers string errorString
        lua_pop(tlModule->L, 5); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * requests request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * requests request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * requests request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 4); // *
        return -1;
    }
    // You need to allocate a new memory to use lua_tostring() because of the garbage collection?? <- but headerValue may not be garbage collected because it is inside headers ?!?!
    *headerValue = lua_tostring(tlModule->L, -1); // requests request headers headerValue
    lua_pop(tlModule->L, 4); // *
    return 0;

}

static int tucube_http_lua_onGetRequestContentLength(lua_State* L) {
    lua_pushstring(L, "contentLength"); // request "contentLength"
    lua_gettable(L, -2); // request contentLength
    return 1;
}

int tucube_epoll_http_Module_onGetRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, ssize_t* contentLength) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_getglobal(tlModule->L, "onGetRequestContentLength"); // onGetRequestContentLength
    if(lua_isnil(tlModule->L, -1)) { // nil 
        lua_pop(tlModule->L, 1); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onGetRequestContentLength); // onGetRequestContentLength
    }
    lua_getglobal(tlModule->L, "requests"); // onGetRequestContentLength requests
    lua_pushinteger(tlModule->L, *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket); // onGetRequestContentLength requests clientSocket
    lua_gettable(tlModule->L, -2); // onGetRequestContentLength requests request
    lua_remove(tlModule->L, -2); // onGetRequestContentLength request
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // requestContentLength
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    if(lua_isnil(tlModule->L, -1)) { // nil 
        lua_pop(tlModule->L, 1); //
        return -1;
    }
    *contentLength = lua_tointeger(tlModule->L, -1); // requestContentLength
    lua_pop(tlModule->L, 1); //
    return 0;
}



int tucube_epoll_http_Module_onResponseStatusCode(struct tucube_Module* module, struct tucube_ClData* clData, int* statusCode) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
    *statusCode = lua_tointeger(tlModule->L, -3); // statusCode headers bodyString
    return 0;
}

int tucube_epoll_http_Module_onResponseHeaderStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_pushnil(tlModule->L); // statusCode headers body nil
    if(lua_next(tlModule->L, -3) != 0) // statusCode headers body newHeaderField newHeaderValue
        return 1; // statusCode headers body newHeaderField newHeaderValue
    return 0; // statusCode headers body
}

int tucube_epoll_http_Module_onResponseHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body newHeaderField newHeaderValue
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    *headerField = lua_tolstring(tlModule->L, -2, headerFieldSize); // statusCode headers body headerField headerValue
    *headerValue = lua_tolstring(tlModule->L, -1, headerValueSize); // statusCode headers body headerField headerValue
    lua_pop(tlModule->L, 1); // statusCode headers body headerField
    if(lua_next(tlModule->L, -3) != 0) // statusCode headers body headerField -> statusCode headers body (newHeaderField newHeaderValue || empty)
        return 1;
    return 0;
}

int tucube_epoll_http_Module_onResponseBodyStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L)); 
    if(lua_isfunction(tlModule->L, -1)) { // statusCode headers bodyFunction
        GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L = lua_newthread(tlModule->L); // statusCode headers bodyFunction thread
        return 1;
    } else if(lua_isstring(tlModule->L, -1)) { // statusCode headers bodyString
        GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L = NULL;
        return 1;
    } else if(lua_isuserdata(tlModule->L, -1)) // statusCode headers bodyFile
        return 1;
    else if(lua_isnil(tlModule->L, -1)) // statusCode headers
        return 0;
    return -1;
}

int tucube_epoll_http_Module_onResponseBody(struct tucube_Module* module, struct tucube_ClData* clData, struct tucube_epoll_http_ResponseBody* responseBody) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    if(lua_isfunction(tlModule->L, -2)) { // statusCode headers bodyFunction thread
        responseBody->type = TUCUBE_EPOLL_HTTP_RESPONSE_BODY_STRING;
        if(lua_gettop(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L) > 1)
            lua_pop(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, 1);
        int coroutineResult;
        lua_pushvalue(tlModule->L, -2); // statusCode headers bodyFunction thread bodyString
        lua_xmove(tlModule->L, GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, 1); // statusCode headers bodyFunction thread
        coroutineResult = lua_resume(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, tlModule->L, 0); // statusCode headers bodyFunction thread
        responseBody->chars = lua_tolstring(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->L, -1, &responseBody->size); // statusCode headers bodyFunction thread
        if(coroutineResult == LUA_YIELD)
            return 1; // statusCode headers bodyFunction thread
        lua_pop(tlModule->L, 1); // statusCode headers body
        if(coroutineResult == 0)
            return 0;
        return -1;
    }
    else if(lua_isstring(tlModule->L, -1)) { // statusCode headers bodyString
        responseBody->type = TUCUBE_EPOLL_HTTP_RESPONSE_BODY_STRING;
        responseBody->chars = lua_tolstring(tlModule->L, -1, &responseBody->size); // statusCode headers bodyString
        return 0;
    } else if(lua_isuserdata(tlModule->L, -1)) { // statusCode headers bodyFile
        // for accurate check, may be you need to compare the metatable of userdata to LUA_FILEHANDLE
        responseBody->type = TUCUBE_EPOLL_HTTP_RESPONSE_BODY_FILE;
        GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->responseBodyStream = lua_touserdata(tlModule->L, -1); // statusCode headers bodyFile
        responseBody->fd = fileno(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->responseBodyStream->f);
        struct stat statBuffer;
        if(fstat(responseBody->fd, &statBuffer) == -1) {
            warn("%s: %u", __FILE__, __LINE__);
            return -1;
        }
        responseBody->size = statBuffer.st_size;
        return 0;
    }
    return -1;
}

int tucube_epoll_http_Module_clDestroy(struct tucube_Module* module, struct tucube_ClData* clData) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
    lua_pop(tlModule->L, lua_gettop(tlModule->L)); //
    close(*GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket);
    *GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->clientSocket = -1;
    if(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->responseBodyStream != NULL) {
        fclose(GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->responseBodyStream->f);
        GON_C_CAST(clData->pointer, struct tucube_http_lua_ClData*)->responseBodyStream->closef = NULL;
    }
    free(clData->pointer);
    free(clData);
    return 0;
}

int tucube_epoll_http_Module_tlDestroy(struct tucube_Module* module) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %d", __FILE__, __LINE__, lua_gettop(tlModule->L));
 
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
