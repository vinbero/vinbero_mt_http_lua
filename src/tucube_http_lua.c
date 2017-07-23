#include <ctype.h>
#include <err.h>
#include <gaio.h>
#include <libgenc/genc_cast.h>
#include <libgenc/genc_list.h>
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
#include <tucube/tucube_IBase.h>
#include <tucube/tucube_ICLocal.h>
#include <tucube/tucube_IHttp.h>

struct tucube_http_lua_TlModule {
   lua_State* L;
};

struct tucube_http_lua_ClData {
    int clientId;
    struct tucube_IHttp_Response* response;
    lua_State* L;
};

TUCUBE_IBASE_FUNCTIONS;
TUCUBE_ICLOCAL_FUNCTIONS;
TUCUBE_IHTTP_FUNCTIONS;

int tucube_IBase_init(struct tucube_Module_Config* moduleConfig, struct tucube_Module_List* moduleList, void* args[]) {
    struct tucube_Module* module = malloc(1 * sizeof(struct tucube_Module));
    GENC_LIST_ELEMENT_INIT(module);
    module->tlModuleKey = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlModuleKey, NULL);
    GENC_LIST_APPEND(moduleList, module);
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

static int tucube_http_lua_writeRaw(lua_State* L) {
    // * response string
    char* bytes;
    size_t bytesSize;
    bytes = lua_tolstring(L, -1, &bytesSize); // * response string
    lua_pushstring(L, "cObject"); // * response string "cObject"
    lua_gettable(L, -3); // * response string cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response string cObject
    response->writeRaw(response, bytes, bytesSize);
    lua_pop(L, 1); // * response string
    return 0;
}

static int tucube_http_lua_writeCrLf(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response cObject
    response->writeCrLf(response);
    lua_pop(L, 1); // * response
    return 0;
}

static int tucube_http_lua_writeVersion(lua_State* L) {
    // * response major minor
    int major = lua_tointeger(L, -2); // * response major minor
    int minor = lua_tointeger(L, -1); // * response major minor
    lua_pushstring(L, "cObject"); // * response major minor "cObject"
    lua_gettable(L, -4); // * response major minor cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response major minor cObject
    response->writeVersion(response, major, minor);
    lua_pop(L, 1); // * response major minor
    return 0;
}

struct int tucube_http_lua_writeStatusCode(lua_State* L) {
    // * response statusCode
    int statusCode = lua_tointeger(L, -1); // * response statusCode
    lua_pushstring(L, "cObject"); // * response statusCode "cObject"
    lua_gettable(L, -3); // * response statusCode cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response statusCode cObject
    lua_pop(L, 1); // * response major minor
    return 0;
}

struct int tucube_http_lua_writeIntHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    int headerValue = lua_tointeger(L, -1); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->writeIntHeader(response, headerField, headerFieldSize, headerValue);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

struct int tucube_http_lua_writeDoubleHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    double headerValue = lua_tonumber(L, -1); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->writeDoubleHeader(response, headerField, headerFieldSize, headerValue);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

struct int tucube_http_lua_writeStringHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    char* headerValue;
    size_t headerValueSize;
    headerValue = lua_tolstring(L, -1, &headerValueSize); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->writeStringHeader(response, headerField, headerFieldSize, headerValue, headerValueSize);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

static int tucube_http_lua_writeBodyString(lua_State* L) {
    // * response string
    char* bodyString;
    size_t bodyStringSize;
    bodyString = lua_tolstring(L, -1, &bodyStringSize); // * response string
    lua_pushstring(L, "cObject"); // * response string "cObject"
    lua_gettable(L, -3); // * response string cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response string cObject
    response->writeRaw(response, bodyString, bodyStringSize);
    lua_pop(L, 1); // * response string
    return 0;
}

static int tucube_http_lua_writeBodyIo(lua_State* L) {
    // * response file
    char* bodyString;
    lua_tolstring(L, -1, &bodyStringSize); // * response string
    lua_pushstring(L, "cObject"); // * response string "cObject"
    lua_gettable(L, -3); // * response string cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response string cObject
    response->writeRaw(response, bodyString, bodyStringSize);
    lua_pop(L, 1); // * response string
    return 0;
}

int tucube_IBase_tlInit(struct tucube_Module* module, struct tucube_Module_Config* moduleConfig, void* args[]) {
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
    lua_newtable(tlModule->L); // tucube
    lua_pushstring(tlModule->L, "args"); // tucube "args"
    if(json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArg") != NULL) {
        lua_pushstring(
            tlModule->L,
            json_string_value(
                json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArgs")
            )
        ); // tucube "args" args
    } else
        lua_pushnil(tlModule->L); // tucube "args" nil
    lua_settable(tlModule->L, -3); // tucube 
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_newtable(tlModule->L); // tucube "requests" requests
    lua_settable(tlModule->L, -3); // tucube
    lua_pushstring(tlModule->L, "responses"); // tucube "responses"
    lua_newtable(tlModule->L); // tucube "responses" responses
    lua_settable(tlModule->L, -3); // tucube
    lua_pushstring(tlModule->L, "responseCallbacks"); // tucube "responseCallbacks"
    lua_newtable(tlModule->L); // tucube "responseCallbacks" reponseCallbacks
    lua_pushstring(tlModule->L, "__index"); // tucube "responseCallbacks" responseCallbacks "__index"
    lua_pushvalue(tlModule->L, -2); // tucube "responseCallbacks" responseCallbacks "__index" responseCallbacks
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeRaw"); // tucube "responseCallbacks" responseCallbacks "writeRaw"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeRaw); // tucube "responseCallbacks" responseCallbacks "writeRaw" writeRaw
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeCrLf"); // tucube "responseCallbacks" responseCallbacks "writeCrLf"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeCrLf); // tucube "responseCallbacks" responseCallbacks "writeCrLf" writeCrLf"
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks 
    lua_pushstring(tlModule->L, "writeVersion"); // tucube "responseCallbacks" responseCallbacks "writeVersion"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeVersion); // tucube "responseCallbacks" responseCallbacks "writeVersion" writeVersion
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeStatusCode"); // tucube "responseCallbacks" responseCallbacks "writeStatusCode"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeStatusCode); // tucube "responseCallbacks" responseCallbacks "writeStatusCode" writeStatusCode
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeIntHeader"); // tucube "responseCallbacks" responseCallbacks "writeIntHeader"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeIntHeader); // tucube "responseCallbacks" responseCallbacks "writeIntHeader" writeIntHeader
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeDoubleHeader"); // tucube "responseCallbacks" responseCallbacks "writeDoubleHeader"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeDoubleHeader); // tucube "responseCallbacks" responseCallbacks "writeDoubleHeader" writeDoubleHeader
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeStringHeader"); // tucube "responseCallbacks" responseCallbacks "writeStringHeader" writeStringHeader
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeStringHeader); // tucube "responseCallbacks" responseCallbacks "writeStringHeader" writeStringHeader
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeBodyString"); // tucube "responseCallbacks" responseCallbacks "writeBodyString"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeBodyString); // tucube "responseCallbacks" responseCallbacks "writeBodyString" writeBodyString
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_pushstring(tlModule->L, "writeBodyIo"); // tucube "responseCallbacks" responseCallbacks "writeBodyIo"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeBodyIo); // tucube "responseCallbacks" responseCallbacks "writeBodyIo" writeBodyIo
    lua_settable(tlModule->L, -3); // tucube "responseCallbacks" responseCallbacks
    lua_settable(tlModule->L, -3); // tucube
    if(luaL_loadfile(tlModule->L, scriptFile) != LUA_OK) { // tucube file
        warnx("%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { // tucube errorString 
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(tlModule->L, 2); //
        pthread_exit(NULL);
    }
    lua_getglobal(tlModule->L, "onInit"); // tucube onInit
    if(lua_isnil(tlModule->L, -1)) // tucube nil
        lua_pop(tlModule->L, 1); // tucube
    else { // tucube onInit
        if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { // tucube
            warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
            lua_pop(tlModule->L, 2); //
	    pthread_exit(NULL);
        }
    }
    lua_setglobal(tlModule->L, "tucube"); //
    pthread_setspecific(*module->tlModuleKey, tlModule);
    return 0;
}

int tucube_ICLocal_init(struct tucube_Module* module, struct tucube_ClData_List* clDataList, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct tucube_ClData* clData = malloc(sizeof(struct tucube_ClData));
    GENC_LIST_ELEMENT_INIT(clData);
    clData->generic.pointer = malloc(1 * sizeof(struct tucube_http_lua_ClData));
    GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId = gaio_Fd_fileno(GENC_CAST(args[0], struct tucube_IHttp_Response*)->io);
    GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->response = args[0];
    GENC_LIST_APPEND(clDataList, clData);
    return 0;
}

int tucube_IHttp_onRequestStart(void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_newtable(tlModule->L); // tucube requests clientId request
    lua_pushstring(tlModule->L, "headers"); // tucube requests clientId request "headers"
    lua_newtable(tlModule->L); // tucube requests clientId request "headers" headers
    lua_settable(tlModule->L, -3); // tucube requests clientId request
    lua_pushstring(tlModule->L, "body"); // tucube requests clientId request "body"
    lua_pushstring(tlModule->L, ""); // tucube requests clientId request "body" ""
    lua_settable(tlModule->L, -3); // tucube requests clientId request
    lua_pushstring(tlModule->L, "contentLength"); // tucube requests clientId request "contentLength"
    lua_pushinteger(tlModule->L, 0); // tucube requests clientId request "contentLength" 0
    lua_settable(tlModule->L, -3); // tucube requests clientId request
    lua_settable(tlModule->L, -3); // tucube requests
    lua_pop(tlModule->L, 1); // tucube
    lua_pushstring(tlModule->L, "responses"); // tucube "responses"
    lua_gettable(tlModule->L, -2); // tucube responses
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube responses clientId
    lua_newtable(tlModule->L); // tucube responses clientId response
    lua_pushstring(tlModule->L, "cObject"); // tucube responses clientId response "cObject"
    lua_pushlightuserdata(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->response); // tucube responses clientId response "cObject" cObject
    lua_settable(tlModule->L, -3); // tucube responses clientId response
    lua_pushstring(tlModule->L, "responseCallbacks"); // tucube responses clientId response "responseCallbacks"
    lua_gettable(tlModule->L, -5); // tucube responses clientId response responseCallbacks
    lua_setmetatable(tlModule->L, -2); // tucube responses clientId response
    lua_pop(tlModule->L, 3); // tucube
    lua_getglobal(tlModule->L, "onRequestStart"); // tucube onRequestStart
    if(lua_isnil(tlModule->L, -1)) { // tucube nil
        lua_pop(tlModule->L, 1); // tucube
        return 0;
    }
    lua_pushstring(tlModule->L, "requests"); // tucube onRequestStart "requests"
    lua_gettable(tlModule->L, -3); // tucube onRequestStart requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube onRequestStart requests clientId
    lua_gettable(tlModule->L, -2); // tucube onRequestStart requests request
    lua_remove(tlModule->L, -2); // tucube onRequestStart request
    lua_pushstring(tlModule->L, "responses"); // tucube onRequestStart request "responses"
    lua_gettable(tlModule->L, -4); // tucube onRequestStart request responses
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube onRequestStart request responses clientId
    lua_gettable(tlModule->L, -2); // tucube onRequestStart request responses response
    lua_remove(tlModule->L, -2); // tucube onRequestStart request response
    if(lua_pcall(tlModule->L, 2, 0, 0) != 0) { // tucube errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(tlModule->L, 2); //
        return -1;
    }
    lua_pop(tlModule->L, 1); //
    return 0;
}

int tucube_IHttp_onRequestMethod(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "method"); // tucube requests request "method"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "method" method 
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); // 
    return 0;
}

int tucube_IHttp_onRequestUri(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "requestUri"); // tucube requests request "requestUri"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "requestUri" requestUri 
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); //
    return 0;
}

int tucube_IHttp_onRequestProtocol(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "protocol"); // tucube requests request "protocol"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "protocol" protocol 
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); //
    return 0;
}

int tucube_IHttp_onRequestContentLength(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "contentLength"); // tucube requests request "contentLength"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "contentLength" contentLength
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); // 
    return 0;
}

int tucube_IHttp_onRequestContentType(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "contentType"); // tucube requests request "contentType"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "contentType" contentType
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); //
    return 0;
}

int tucube_IHttp_onRequestScriptPath(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "scriptPath"); // tucube requests request "scriptPath"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "scriptPath" scriptPath
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); // 
    return 0;
}

int tucube_IHttp_onRequestHeaderField(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // tucube requests request "recentHeaderField"
    for(ssize_t index = 0; index < tokenSize; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request "recentHeaderField" recentHeaderField
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); //
    return 0;
}
int tucube_IHttp_onRequestHeaderValue(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // tucube requests request "recentHeaderField"
    lua_gettable(tlModule->L, -2); // tucube requests request recentHeaderField
    lua_pushvalue(tlModule->L, -2); // tucube requests request recentHeaderField request
    lua_pushstring(tlModule->L, "headers"); // tucube requests request recentHeaderField request "headers"
    lua_gettable(tlModule->L, -2); // tucube requests request recentHeaderField request headers
    lua_pushvalue(tlModule->L, -3); // tucube requests request recentHeaderField request headers requestHeaderField
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube requests request recentHeaderField request headers recentHeaderField token
    lua_settable(tlModule->L, -3); // tucube requests request recentHeaderField request headers
    lua_pop(tlModule->L, 3); // tucube requests request
    lua_pushstring(tlModule->L, "recentHeaderField"); // tucube requests request "recentHeaderField"
    lua_pushnil(tlModule->L); // tucube requests request "recentHeaderField" nil
    lua_settable(tlModule->L, -3); // tucube requests request
    lua_pop(tlModule->L, 3); //
    return 0;
}

int tucube_IHttp_onRequestHeadersFinish(void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_getglobal(tlModule->L, "onRequestHeadersFinish"); // tucube onRequestHeadersFinish
    if(lua_isnil(tlModule->L, -1)) {
        lua_pop(tlModule->L, 2); // tucube
        return 0;
    }
    lua_pushstring(tlModule->L, "requests"); // tucube onRequestHeadersFinish "requests"
    lua_gettable(tlModule->L, -3); // tucube onRequestHeadersFinish requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube onRequestHeadersFinish requests clientId
    lua_gettable(tlModule->L, -2); // tucube onRequestHeadersFinish requests request
    lua_remove(tlModule->L, -2); // tucube onRequestHeadersFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(tlModule->L, 2); //
        return -1;
    }
    lua_pop(tlModule->L, 1); //
    return 0;
}

static int tucube_http_lua_onRequestBodyStart(lua_State* L) {
    // * request
    lua_pushstring(L, "body"); // * request "body"
    lua_newtable(L); // * request "body" body
    lua_settable(L, -3); // * request
    return 0;
}

int tucube_IHttp_onRequestBodyStart(void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_getglobal(tlModule->L, "onRequestBodyStart"); // tucube onRequestBodyStart
    if(lua_isnil(tlModule->L, -1)) { // tucube nil
        lua_pop(tlModule->L, 1); // tucube
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyStart); // tucube onRequestBodyStart
    }
    lua_pushstring(tlModule->L, "requests"); // tucube onRequestBodyStart "requests"
    lua_gettable(tlModule->L, -3); // tucube onRequestBodyStart requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube onRequestBodyStart requests clientId
    lua_gettable(tlModule->L, -2); // tucube onRequestBodyStart requests request
    lua_remove(tlModule->L, -2); // tucube onRequestBodyStart request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(tlModule->L, 2); //
	return -1;
    }
    lua_pop(tlModule->L, 1); //
    return 0;
}

static int tucube_http_lua_onRequestBody(lua_State* L) {
    // * request bodyChunk
    lua_getglobal(L, "table"); // * request bodyChunk table
    lua_pushstring(L, "insert"); // * request bodyChunk table "insert"
    lua_gettable(L, -2); // * request bodyChunk table insert
    lua_pushvalue(L, 1); // * request bodyChunk table insert request
    lua_pushstring(L, "body"); // * request bodyChunk table insert request "body"
    lua_gettable(L, -2); // * request bodyChunk table insert request body
    lua_remove(L, -2); // * request bodyChunk table insert body
    lua_pushvalue(L, 2); // * request bodyChunk table insert body bodyChunk
    if(lua_pcall(L, 2, 0, 0) != 0) { // * request bodyChunk table errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(L, 3); // * request
        luaL_error("tucube_http_lua_onRequestBody() failed"); //
    }
    lua_pop(L, 1); // * request bodyChunk
    return 0;
}

int tucube_IHttp_onRequestBody(char* bodyChunk, ssize_t bodyChunkSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_getglobal(tlModule->L, "onRequestBody"); // tucube requests onRequestBody
    if(lua_isnil(tlModule->L, -1)) { // tucube requests nil
        lua_pop(tlModule->L, 1); //  tucube requests
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBody); // tucube requests onRequestBody
    }
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests onRequestBody clientId
    lua_gettable(tlModule->L, -3); // tucube requests onRequestBody request
    lua_pushlstring(tlModule->L, bodyChunk, bodyChunkSize); // tucube requests onRequestBody request bodyChunk
    if(lua_pcall(tlModule->L, 2, 0, 0) != 0) { // tucube requests errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube requests errorString
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    return 0;
}
static int tucube_http_lua_onRequestBodyFinish(lua_State* L) {
    // * request
    lua_pushstring(L, "body"); // * request "body"
    lua_gettable(L, -2); // * request body
    lua_getglobal(L, "table"); // * request body table
    lua_pushstring(L, "concat"); // * request body table "concat"
    lua_gettable(L, -2); // * request body table concat
    lua_pushvalue(L, -3); // * request body table concat body
    if(lua_pcall(L, 1, 1, 0) != 0) { // * request body table errorString
        warnx("%s: %u, %s", __FILE__, __LINE__, lua_tostring(L, -1)); // * request body table errorString
        lua_pop(L, 3); // * request
	luaL_error(L, "tucube_http_lua_onRequestBodyFinish() failed");
    }
    lua_pushvalue(L, -4); // * request body table result request
    lua_pushstring(L, "body"); // * request body table result request "body"
    lua_pushvalue(L, -3); // * request body table result request "body" result
    lua_settable(L, -3); // * request body table result request
    lua_pop(L, 4); // * request
    return 0;
}

int tucube_IHttp_onRequestBodyFinish(void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_getglobal(tlModule->L, "onRequestBodyFinish"); // tucube onRequestBodyFinish
    if(lua_isnil(tlModule->L, -1)) { // nil
        lua_pop(tlModule->L, 1); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyFinish); // tucube onRequestBodyFinish
    } 
    lua_pushstring(tlModule->L, "requests"); // tucube onRequestBodyFinsih "requests"
    lua_gettable(tlModule->L, -3); // tucube onRequestBodyFinish requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube onRequestBodyFinish requests clientId
    lua_gettable(tlModule->L, -2); // tucube onRequestBodyFinish requests request
    lua_remove(tlModule->L, -2); // tucube onRequestBodyFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(tlModule->L, 2); //
        return -1;
    }
    lua_pop(tlModule->L, 1); //
    return 0;
}

int tucube_IHttp_onGetRequestIntHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, int* headerValue) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "requests"); // * tucube "requests"
    lua_gettable(tlModule->L, -2); // * tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube requests clientId
    lua_gettable(tlModule->L, -2); // * tucube requests request
    lua_pushstring(tlModule->L, "headers"); // * tucube requests request "headers"
    lua_gettable(tlModule->L, -2); // * tucube requests request headers
    lua_getglobal(tlModule->L, "string"); // * tucube requests request headers string
    lua_pushstring(tlModule->L, "upper"); // * tucube requests request headers string "upper"
    lua_gettable(tlModule->L, -2); // * tucube requests request headers string upper
    lua_pushstring(tlModule->L, headerField); // * tucube requests request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube requests request headers string errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube requests request headers string errorString
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * tucube requests request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * tucube requests request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * tucube requests request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 5); // *
        return -1;
    }
    *headerValue = lua_tointeger(tlModule->L, -1); // * tucube requests request headers headerValue;
    lua_pop(tlModule->L, 5); // *
    return 0;
}
int tucube_IHttp_onGetRequestDoubleHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, double* headerValue) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "requests"); // * tucube "requests"
    lua_gettable(tlModule->L, -2); // * tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube requests clientId
    lua_gettable(tlModule->L, -2); // * tucube requests request
    lua_pushstring(tlModule->L, "headers"); // * tucube requests request "headers"
    lua_gettable(tlModule->L, -2); // * tucube requests request headers
    lua_getglobal(tlModule->L, "string"); // * tucube requests request headers string
    lua_pushstring(tlModule->L, "upper"); // * tucube requests request headers string "upper"
    lua_gettable(tlModule->L, -2); // * tucube requests request headers string upper
    lua_pushstring(tlModule->L, headerField); // * tucube requests request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube requests request headers string upperedHeaderField
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube requests request headers string errorString
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * tucube requests request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * tucube requests request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * tucube requests request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 5); // *
        return -1;
    }
    *headerValue = lua_tonumber(tlModule->L, -1); // * tucube requests request headers headerValue;
    lua_pop(tlModule->L, 5); // * 
    return 0;
}

int tucube_IHttp_onGetRequestStringHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, const char** headerValue) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "requests"); // * tucube "requests"
    lua_gettable(tlModule->L, -2); // * tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube requests clientId
    lua_gettable(tlModule->L, -2); // * tucube requests request
    lua_pushstring(tlModule->L, "headers"); // * tucube requests request "headers"
    lua_gettable(tlModule->L, -2); // * tucube requests request headers
    lua_getglobal(tlModule->L, "string"); // * tucube requests request headers string
    lua_pushstring(tlModule->L, "upper"); // * tucube requests request headers string "upper"
    lua_gettable(tlModule->L, -2); // * tucube requests request headers string upper
    lua_pushstring(tlModule->L, headerField); // * tucube requests request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube requests request headers string errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube requests request headers string errorString
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * tucube requests request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * tucube requests request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * tucube requests request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 5); // *
        return -1;
    }
    // You need to allocate a new memory to use lua_tostring() because of the garbage collection?? <- but headerValue may not be garbage collected because it is inside headers ?!?!
    *headerValue = lua_tostring(tlModule->L, -1); // * tucube requests request headers headerValue
    lua_pop(tlModule->L, 5); // *
    return 0;

}

static int tucube_http_lua_onGetRequestContentLength(lua_State* L) {
    lua_pushstring(L, "contentLength"); // * request "contentLength"
    lua_gettable(L, -2); // * request contentLength
    return 1;
}

int tucube_IHttp_onGetRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, ssize_t* contentLength) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L "tucube"); // * tucube
    lua_getglobal(tlModule->L, "onGetRequestContentLength"); // * tucube onGetRequestContentLength
    if(lua_isnil(tlModule->L, -1)) { // * tucube nil 
        lua_pop(tlModule->L, 2); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onGetRequestContentLength); // * tucube onGetRequestContentLength
    }
    lua_pushstring(tlModule->L, "requests"); // * tucube onGetRequestContentLength "requests"
    lua_gettable(tlModule->L, -3); // * tucube onGetRequestContentLength requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube onGetRequestContentLength requests clientId
    lua_gettable(tlModule->L, -2); // * tucube onGetRequestContentLength requests request
    lua_remove(tlModule->L, -2); // * tucube onGetRequestContentLength request
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube errorString
        lua_pop(tlModule->L, 2); //
        return -1;
    }
    if(lua_isnil(tlModule->L, -1)) { // * tucube nil 
        lua_pop(tlModule->L, 2); // *
        return -1;
    }
    *contentLength = lua_tointeger(tlModule->L, -1); // * tucube requestContentLength
    lua_pop(tlModule->L, 2); // *
    return 0;
}

int tucube_IHttp_onRequestFinish(struct tucube_Module* module, struct tucube_ClData* clData, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "requests"); // tucube "requests"
    lua_gettable(tlModule->L, -2); // tucube requests
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube requests clientId
    lua_gettable(tlModule->L, -2); // tucube requests request
    const char* requestUri;
    const char* scriptPath;
    size_t scriptPathSize;
    const char* pathInfo;
    const char* queryString;
    lua_pushstring(tlModule->L, "requestUri"); // tucube requests request "requestUri"
    lua_gettable(tlModule->L, -2); // tucube requests request requestUri
    requestUri = lua_tostring(tlModule->L, -1); // tucube requests request requestUri
    lua_pop(tlModule->L, 1); // tucube requests request
    lua_pushstring(tlModule->L, "scriptPath"); // tucube requests request "scriptPath"
    lua_gettable(tlModule->L, -2); // tucube requests request scriptPath
    if(lua_isnil(tlModule->L, -1)) { // tucube requests request nil
        lua_pop(tlModule->L, 1); // tucube requests request
        lua_pushstring(tlModule->L, "scriptPath"); // tucube requests request "scriptPath"
        scriptPath = "";
        scriptPathSize = sizeof("") - 1;
        lua_pushstring(tlModule->L, ""); // tucube requests request "scriptPath" "" 
        lua_settable(tlModule->L, -3); // tucube requests request
    }
    else { // tucube requests request scriptPath
        scriptPath = lua_tolstring(tlModule->L, -1, &scriptPathSize); // tucube requests request scriptPath 
        if(strncmp(scriptPath + scriptPathSize - 1, "/", sizeof("/") - 1) == 0) { // tucube requests request scriptPath
            warnx("%s: %u: scriptPath should not  end with /", __FILE__, __LINE__);
            lua_pop(tlModule->L, 4); //
            return -1;
        }
        lua_pop(tlModule->L, 1); // tucube requests request
    }
    if((pathInfo = strstr(requestUri, scriptPath)) != requestUri) { // tucube requests request // if request uri doesn't begin with script name
        warnx("%s: %u: Request uri doesn't begin with script name", __FILE__, __LINE__);
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    pathInfo += scriptPathSize;
    if((queryString = strstr(pathInfo, "?")) != NULL) { // check if there is query string
        ++queryString; // query string begins after the question mark
        if(strstr(queryString, "?") != NULL) { // check if there is unnecessary question mark after query string
            warnx("%s: %u: Unnecessary question mark after query string", __FILE__, __LINE__);
            lua_pop(tlModule->L, 3); //
            return -1;
        }
 
        lua_pushstring(tlModule->L, "pathInfo"); // tucube requests request "pathInfo"
        if(queryString - pathInfo - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlModule->L, pathInfo, queryString - pathInfo - 1); // tucube requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // tucube requests request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // tucube requests request
        lua_pushstring(tlModule->L, "queryString"); // tucube requests request "queryString"
        lua_pushstring(tlModule->L, queryString); // tucube requests request "queryString" queryString 
        lua_settable(tlModule->L, -3); // tucube requests request 
    }
    else {
        lua_pushstring(tlModule->L, "queryString"); // tucube requests request "queryString"
        lua_pushstring(tlModule->L, ""); // tucube requests request "queryString" ""
        lua_settable(tlModule->L, -3); // tucube requests request
        lua_pushstring(tlModule->L, "pathInfo"); // tucube requests request "pathInfo"
        if(strlen(pathInfo) != 0) // check if path info is not empty string
            lua_pushstring(tlModule->L, pathInfo); // tucube requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // tucube requests request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // tucube requests request
    }
    lua_getglobal(tlModule->L, "onRequestFinish"); // tucube requests request onRequestFinish
    if(lua_isnil(tlModule->L, -1)) { // tucube requests request nil
        warnx("%s: %u: onRequestFinish() is not found in the script", __FILE__, __LINE__);
        lua_pop(tlModule->L, 4); //
        return -1;
    }
    lua_pushvalue(tlModule->L, -2); // tucube requests request onRequestFinish request
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube requests request errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube requests request errorString
        lua_pop(tlModule->L, 4); //
        return -1;
    }

    lua_pop(tlModule->L, 3); //
/*
    lua_remove(tlModule->L, 1); // tucube request statusCode headers body
    lua_remove(tlModule->L, 1); // tucube statusCode headers body
    if(!(lua_isnumber(tlModule->L, -3) && lua_istable(tlModule->L, -2) && (lua_isstring(tlModule->L, -1) || lua_isfunction(tlModule->L, -1) || lua_isnil(tlModule->L, -1) || lua_isuserdata(tlModule->L, -1)))) {
        lua_pop(tlModule->L, lua_gettop(tlModule->L)); //
        lua_pushinteger(tlModule->L, 500); // 500
        lua_newtable(tlModule->L); // 500 headers
        lua_pushstring(tlModule->L, "Content-Type"); // 500 headers "Content-Type"
        lua_pushstring(tlModule->L, "text/plain; charset=utf8"); // 500 headers "Content-Type" "text/plain; charset=utf8"
        lua_settable(tlModule->L, -3); // 500 headers 
        lua_pushstring(tlModule->L, "500 Internal Server Error"); // 500 headers "Internal Server Error"
    }
*/
    return 0;
}
/*
int tucube_IHttp_onResponseStatusCode(struct tucube_Module* module, struct tucube_ClData* clData, int* statusCode) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    *statusCode = lua_tointeger(tlModule->L, -3); // statusCode headers bodyString
    return 0;
}

int tucube_IHttp_onResponseHeaderStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body
    lua_pushnil(tlModule->L); // statusCode headers body nil
    if(lua_next(tlModule->L, -3) != 0) // statusCode headers body newHeaderField newHeaderValue
        return 1; // statusCode headers body newHeaderField newHeaderValue
    return 0; // statusCode headers body
}

int tucube_IHttp_onResponseHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char** headerField, size_t* headerFieldSize, const char** headerValue, size_t* headerValueSize) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body newHeaderField newHeaderValue
warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    *headerField = lua_tolstring(tlModule->L, -2, headerFieldSize); // statusCode headers body headerField headerValue
    *headerValue = lua_tolstring(tlModule->L, -1, headerValueSize); // statusCode headers body headerField headerValue
    lua_pop(tlModule->L, 1); // statusCode headers body headerField
    if(lua_next(tlModule->L, -3) != 0) // statusCode headers body headerField -> statusCode headers body (newHeaderField newHeaderValue || empty)
        return 1;
    return 0;
}

int tucube_IHttp_onResponseBodyStart(struct tucube_Module* module, struct tucube_ClData* clData) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__); 
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body
    if(lua_isfunction(tlModule->L, -1)) { // statusCode headers bodyFunction
        GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L = lua_newthread(tlModule->L); // statusCode headers bodyFunction thread
        return 1;
    } else if(lua_isstring(tlModule->L, -1)) { // statusCode headers bodyString
        GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L = NULL;
        return 1;
    } else if(lua_isuserdata(tlModule->L, -1)) // statusCode headers bodyFile
        return 1;
    else if(lua_isnil(tlModule->L, -1)) // statusCode headers
        return 0;
    return -1;
}
int tucube_IHttp_onResponseBody(struct tucube_Module* module, struct tucube_ClData* clData, struct tucube_epoll_http_ResponseBody* responseBody) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    // statusCode headers body
    if(lua_isfunction(tlModule->L, -2)) { // statusCode headers bodyFunction thread
        responseBody->type = TUCUBE_EPOLL_HTTP_RESPONSE_BODY_STRING;
        if(lua_gettop(GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L) > 1)
            lua_pop(GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L, 1);
        int coroutineResult;
        lua_pushvalue(tlModule->L, -2); // statusCode headers bodyFunction thread bodyString
        lua_xmove(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L, 1); // statusCode headers bodyFunction thread
        coroutineResult = lua_resume(GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L, tlModule->L, 0); // statusCode headers bodyFunction thread
        responseBody->chars = lua_tolstring(GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->L, -1, &responseBody->size); // statusCode headers bodyFunction thread
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
        GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->responseBodyStream = lua_touserdata(tlModule->L, -1); // statusCode headers bodyFile
        responseBody->fd = fileno(GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->responseBodyStream->f);
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
*/


int tucube_ICLocal_destroy(struct tucube_Module* module, struct tucube_ClData* clData) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    free(clData->generic.pointer);
    free(clData);
    return 0;
}

int tucube_IBase_tlDestroy(struct tucube_Module* module) {
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
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

int tucube_IBase_destroy(struct tucube_Module* module) {
warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    pthread_key_delete(*module->tlModuleKey);
    free(module->tlModuleKey);
    free(module);
    return 0;
}
