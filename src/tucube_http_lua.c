#include <assert.h>
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
    json_t* scriptArgs = json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArgs");
    if(scriptArgs != NULL && json_is_object(scriptArgs)) {
        json_object_set_new(
            json_array_get(moduleConfig->json, 1),
            "tucube_http_lua.scriptArgs",
            json_string(json_dumps(scriptArgs, 0))
        );
    }
    return 0;
}

static int tucube_http_lua_writeBytes(lua_State* L) {
    // * response bytes 
    char* bytes;
    size_t bytesSize;
    bytes = lua_tolstring(L, -1, &bytesSize); // * response bytess
    lua_pushstring(L, "cObject"); // * bytes string "cObject"
    lua_gettable(L, -3); // * response bytes cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response bytes cObject
    response->methods->writeBytes(response, bytes, bytesSize);
    lua_pop(L, 1); // * response bytes 
    return 0;
}

static int tucube_http_lua_writeIo(lua_State* L) {
    // * response file
    luaL_Stream* bodyFile = lua_touserdata(L, -1); // * response file 
    lua_pushstring(L, "cObject"); // * response file "cObject"
    lua_gettable(L, -3); // * response file cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response file cObject
    struct gaio_Io io;
    struct gaio_Io_Methods ioMethods;
    GAIO_NOP_INIT(&ioMethods);
    ioMethods.read = gaio_Fd_read;
    io.object.integer = fileno(bodyFile->f);
    io.methods = &ioMethods;
    struct stat statBuffer;
    fstat(io.object.integer, &statBuffer);
    response->methods->writeIo(response, &io, statBuffer.st_size);
    fclose(bodyFile->f);
    bodyFile->closef = NULL;
    lua_pop(L, 1); // * response file 
    return 0;
}

static int tucube_http_lua_writeCrLf(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response cObject
    response->methods->writeCrLf(response);
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
    response->methods->writeVersion(response, major, minor);
    lua_pop(L, 1); // * response major minor
    return 0;
}

static int tucube_http_lua_writeStatusCode(lua_State* L) {
    // * response statusCode
    int statusCode = lua_tointeger(L, -1); // * response statusCode
    lua_pushstring(L, "cObject"); // * response statusCode "cObject"
    lua_gettable(L, -3); // * response statusCode cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response statusCode cObject
    response->methods->writeStatusCode(response, statusCode);
    lua_pop(L, 1); // * response statusCode
    return 0;
}

static int tucube_http_lua_writeIntHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    int headerValue = lua_tointeger(L, -1); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->methods->writeIntHeader(response, headerField, headerFieldSize, headerValue);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

static int tucube_http_lua_writeDoubleHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    double headerValue = lua_tonumber(L, -1); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct tucube_IHttp_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->methods->writeDoubleHeader(response, headerField, headerFieldSize, headerValue);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

static int tucube_http_lua_writeStringHeader(lua_State* L) {
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
    response->methods->writeStringHeader(response, headerField, headerFieldSize, headerValue, headerValueSize);
    lua_pop(L, 1); // * response headerField headerValue
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
    if(json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArgs") != NULL) {
        lua_pushstring(
            tlModule->L,
            json_string_value(
                json_object_get(json_array_get(moduleConfig->json, 1), "tucube_http_lua.scriptArgs")
            )
        ); // tucube "args" args
    } else
        lua_pushnil(tlModule->L); // tucube "args" nil
    lua_settable(tlModule->L, -3); // tucube 
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_newtable(tlModule->L); // tucube "clients" clients
    lua_settable(tlModule->L, -3); // tucube
    lua_pushstring(tlModule->L, "responseMethods"); // tucube "responseMethods"
    lua_newtable(tlModule->L); // tucube "responseMethods" reponseMethods
    lua_pushstring(tlModule->L, "__index"); // tucube "responseMethods" responseMethods "__index"
    lua_pushvalue(tlModule->L, -2); // tucube "responseMethods" responseMethods "__index" responseMethods
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeBytes"); // tucube "responseMethods" responseMethods "writeBytes"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeBytes); // tucube "responseMethods" responseMethods "writeBytes" writeBytes
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeIo"); // tucube "responseMethods" responseMethods "writeIo"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeIo); // tucube "responseMethods" responseMethods "writeIo" writeIo
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeCrLf"); // tucube "responseMethods" responseMethods "writeCrLf"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeCrLf); // tucube "responseMethods" responseMethods "writeCrLf" writeCrLf"
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods 
    lua_pushstring(tlModule->L, "writeVersion"); // tucube "responseMethods" responseMethods "writeVersion"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeVersion); // tucube "responseMethods" responseMethods "writeVersion" writeVersion
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeStatusCode"); // tucube "responseMethods" responseMethods "writeStatusCode"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeStatusCode); // tucube "responseMethods" responseMethods "writeStatusCode" writeStatusCode
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeIntHeader"); // tucube "responseMethods" responseMethods "writeIntHeader"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeIntHeader); // tucube "responseMethods" responseMethods "writeIntHeader" writeIntHeader
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeDoubleHeader"); // tucube "responseMethods" responseMethods "writeDoubleHeader"
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeDoubleHeader); // tucube "responseMethods" responseMethods "writeDoubleHeader" writeDoubleHeader
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeStringHeader"); // tucube "responseMethods" responseMethods "writeStringHeader" writeStringHeader
    lua_pushcfunction(tlModule->L, tucube_http_lua_writeStringHeader); // tucube "responseMethods" responseMethods "writeStringHeader" writeStringHeader
    lua_settable(tlModule->L, -3); // tucube "responseMethods" responseMethods
    lua_settable(tlModule->L, -3); // tucube
    lua_setglobal(tlModule->L, "tucube"); //
    if(luaL_loadfile(tlModule->L, scriptFile) != LUA_OK) { // chunk
        warnx("%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { // errorString 
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        pthread_exit(NULL);
    }
    lua_getglobal(tlModule->L, "onInit"); // onInit
    if(lua_isnil(tlModule->L, -1)) // nil
        lua_pop(tlModule->L, 1); //
    else { // onInit
        if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { //
            warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // errorString
            lua_pop(tlModule->L, 1); //
	    pthread_exit(NULL);
        }
    }
    assert(lua_gettop(tlModule->L) == 0);
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
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_newtable(tlModule->L); // tucube clients clientId client
    lua_pushstring(tlModule->L, "request"); // tucube clients clientId client "request"
    lua_newtable(tlModule->L); // tucube clients clientId client "request" request
    lua_pushstring(tlModule->L, "headers"); // tucube clients clientId client "request" request "headers"
    lua_newtable(tlModule->L); // tucube clients clientId client "request" request "headers" headers
    lua_settable(tlModule->L, -3); // tucube clients clientId client "request" request
    lua_pushstring(tlModule->L, "body"); // tucube clients clientId client "request" request "body"
    lua_pushstring(tlModule->L, ""); // tucube clients clientId client "request" request "body" ""
    lua_settable(tlModule->L, -3); // tucube clients clientId client "request" request
    lua_pushstring(tlModule->L, "contentLength"); // tucube clients clientId client "request" request "contentLength"
    lua_pushinteger(tlModule->L, 0); // tucube clients clientId client "request" request "contentLength" 0
    lua_settable(tlModule->L, -3); // tucube clients clientId client "request" request
    lua_settable(tlModule->L, -3); // tucube clients clientId client
    lua_pushstring(tlModule->L, "response"); // tucube clients clientId client "response"
    lua_newtable(tlModule->L); // tucube clients clientId client "response" response
    lua_pushstring(tlModule->L, "cObject"); // tucube clients clientId client "response" response "cObject"
    lua_pushlightuserdata(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->response); // tucube clients clientId client "response" response "cObject" cObject
    lua_settable(tlModule->L, -3); // tucube clients clientId client "response" response
    lua_pushstring(tlModule->L, "responseMethods"); // tucube clients clientId client "response" response "responseMethods"
    lua_gettable(tlModule->L, -7); // tucube clients clientId client "response" response responseMethods
    lua_setmetatable(tlModule->L, -2); // tucube clients clientId client "response" response
    lua_settable(tlModule->L, -3); // tucube clients clientId client
    lua_settable(tlModule->L, -3); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_remove(tlModule->L, -2); // tucube client
    lua_getglobal(tlModule->L, "onRequestStart"); // tucube client onRequestStart
    if(lua_isnil(tlModule->L, -1)) { // tucube client nil
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return 0;
    }
    lua_pushvalue(tlModule->L, -2); // tucube client onRequestStart client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube client errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube client errorString
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestMethod(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "method"); // tucube clients client request "method"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request "method" method 
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); // 
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestUri(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "requestUri"); // tucube clients client request "requestUri"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request "requestUri" requestUri 
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestVersionMajor(int major, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "versionMajor"); // tucube clients client request "versionMajor"
    lua_pushinteger(tlModule->L, major); // tucube clients client request "versionMajor" versionMajor 
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestVersionMinor(int minor, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "versionMinor"); // tucube clients client request "versionMinor"
    lua_pushinteger(tlModule->L, minor); // tucube clients client request "versionMinor" versionMinor
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestContentLength(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "contentLength"); // tucube clients client request "contentLength"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request "contentLength" contentLength
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); // 
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestContentType(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "contentType"); // tucube clients client request "contentType"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request "contentType" contentType
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestScriptPath(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clientss clientId
    lua_gettable(tlModule->L, -2); // tucube clients client 
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "scriptPath"); // tucube clients client request "scriptPath"
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request "scriptPath" scriptPath
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestHeaderField(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "recentHeaderField"); // tucube clients client request "recentHeaderField"
    for(ssize_t index = 0; index < tokenSize; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request "recentHeaderField" recentHeaderField
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestHeaderValue(char* token, ssize_t tokenSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    lua_pushstring(tlModule->L, "recentHeaderField"); // tucube clients client request "recentHeaderField"
    lua_gettable(tlModule->L, -2); // tucube clients client request recentHeaderField
    lua_pushvalue(tlModule->L, -2); // tucube clients client request recentHeaderField request
    lua_pushstring(tlModule->L, "headers"); // tucube clients client request recentHeaderField request "headers"
    lua_gettable(tlModule->L, -2); // tucube clients client request recentHeaderField request headers
    lua_pushvalue(tlModule->L, -3); // tucube clients client request recentHeaderField request headers requestHeaderField
    lua_pushlstring(tlModule->L, token, tokenSize); // tucube clients client request recentHeaderField request headers recentHeaderField token
    lua_settable(tlModule->L, -3); // tucube clients client request recentHeaderField request headers
    lua_pop(tlModule->L, 3); // tucube clients client request
    lua_pushstring(tlModule->L, "recentHeaderField"); // tucube clients client request "recentHeaderField"
    lua_pushnil(tlModule->L); // tucube clients client request "recentHeaderField" nil
    lua_settable(tlModule->L, -3); // tucube clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onRequestHeadersFinish(void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_getglobal(tlModule->L, "onRequestHeadersFinish"); // tucube clients client onRequestHeadersFinish
    if(lua_isnil(tlModule->L, -1)) { // tucube clients client nil
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return 0;
    }
    lua_pushvalue(tlModule->L, -2); // tucube clients client onRequestHeadersFinish client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube clients client errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1));
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 1); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

static int tucube_http_lua_onRequestBodyStart(lua_State* L) {
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "body"); // * client request "body"
    lua_newtable(L); // * client request "body" body
    lua_settable(L, -3); // * client request
    lua_pop(L, 1); // * client
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
    lua_pushstring(tlModule->L, "responses"); // tucube onRequestBodyStart request "responses"
    lua_gettable(tlModule->L, -4); // tucube onRequestBodyStart request responses
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube onRequestBodyStart request responses clientId
    lua_gettable(tlModule->L, -2); // tucube onRequestBodyStart request responses response
    lua_remove(tlModule->L, -2); // tucube onRequestBodyStart request response
    if(lua_pcall(tlModule->L, 2, 0, 0) != 0) { // tucube errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube errorString
        lua_pop(tlModule->L, 2); //
        assert(lua_gettop(tlModule->L) == 0);
	return -1;
    }
    lua_pop(tlModule->L, 1); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

static int tucube_http_lua_onRequestBody(lua_State* L) {
    // * client bodyChunk
    lua_pushstring(L, "request"); // * client bodyChunk "request"
    lua_gettable(L, -3); // * client bodyChunk request
    lua_getglobal(L, "table"); // * client bodyChunk request table
    lua_pushstring(L, "insert"); // * client bodyChunk request table "insert"
    lua_gettable(L, -2); // * client bodyChunk request table insert
    lua_pushvalue(L, -3); // * client bodyChunk request table insert request
    lua_pushstring(L, "body"); // * client bodyChunk request table insert request "body"
    lua_gettable(L, -2); // * client bodyChunk request table insert request body
    lua_remove(L, -2); // * client bodyChunk request table insert body
    lua_pushvalue(L, -5); // * client bodyChunk request table insert body bodyChunk
    if(lua_pcall(L, 2, 0, 0) != 0) { // * client bodyChunk request table errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(L, -1));
        lua_pop(L, 3); // * client bodyChunk
        luaL_error(L, "tucube_http_lua_onRequestBody() failed"); //
    }
    lua_pop(L, 2); // * client bodyChunk
    return 0;
}

int tucube_IHttp_onRequestBody(char* bodyChunk, ssize_t bodyChunkSize, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_getglobal(tlModule->L, "onRequestBody"); // tucube clients onRequestBody
    if(lua_isnil(tlModule->L, -1)) { // tucube clients nil
        lua_pop(tlModule->L, 1); //  tucube clients
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBody); // tucube clients onRequestBody
    }
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients onRequestBody clientId
    lua_gettable(tlModule->L, -3); // tucube clients onRequestBody client
    lua_pushlstring(tlModule->L, bodyChunk, bodyChunkSize); // tucube clients onRequestBody client bodyChunk
    if(lua_pcall(tlModule->L, 2, 0, 0) != 0) { // tucube clients errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube clients errorString
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}
static int tucube_http_lua_onRequestBodyFinish(lua_State* L) {
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "body"); // * client request "body"
    lua_gettable(L, -2); // * client request body
    lua_getglobal(L, "table"); // * client request body table
    lua_pushstring(L, "concat"); // * client request body table "concat"
    lua_gettable(L, -2); // * client request body table concat
    lua_pushvalue(L, -3); // * client request body table concat body
    if(lua_pcall(L, 1, 1, 0) != 0) { // * client request body table errorString
        warnx("%s: %u, %s", __FILE__, __LINE__, lua_tostring(L, -1)); // * client request body table errorString
        lua_pop(L, 4); // * client
	luaL_error(L, "tucube_http_lua_onRequestBodyFinish() failed");
    }
    lua_pushvalue(L, -4); // * client request body table result request
    lua_pushstring(L, "body"); // * client request body table result request "body"
    lua_pushvalue(L, -3); // * client request body table result request "body" result
    lua_settable(L, -3); // * client request body table result request
    lua_pop(L, 5); // * client
    return 0;
}

int tucube_IHttp_onRequestBodyFinish(void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_Module* module = args[0];
    struct tucube_ClData* clData = args[1];
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_getglobal(tlModule->L, "onRequestBodyFinish"); // tucube clients onRequestBodyFinish
    if(lua_isnil(tlModule->L, -1)) { // tucube clients nil
        lua_pop(tlModule->L, 1); // tucube clients
        lua_pushcfunction(tlModule->L, tucube_http_lua_onRequestBodyFinish); // tucube clients onRequestBodyFinish
    } 
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients onRequestBodyFinish clientId
    lua_gettable(tlModule->L, -3); // tucube clients onRequestBodyFinish client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube clients errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // tucube clients errorString
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IHttp_onGetRequestIntHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, int* headerValue) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "clients"); // * tucube "clients"
    lua_gettable(tlModule->L, -2); // * tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube clients clientId
    lua_gettable(tlModule->L, -2); // * tucube clients client 
    lua_pushstring(tlModule->L, "request"); // * tucube clients client "request"
    lua_gettable(tlModule->L, -2); // * tucube clients client request
    lua_pushstring(tlModule->L, "headers"); // * tucube clients client request "headers"
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers
    lua_getglobal(tlModule->L, "string"); // * tucube clients client request headers string
    lua_pushstring(tlModule->L, "upper"); // * tucube clients client request headers string "upper"
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers string upper
    lua_pushstring(tlModule->L, headerField); // * tucube clients client request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube clients client request headers string errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube clients client request headers string errorString
        lua_pop(tlModule->L, 7); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * tucube clients client request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * tucube clients client request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tointeger(tlModule->L, -1);
    lua_pop(tlModule->L, 6); // *
    return 0;
}
int tucube_IHttp_onGetRequestDoubleHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, double* headerValue) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "clients"); // * tucube "clients"
    lua_gettable(tlModule->L, -2); // * tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube clients clientId
    lua_gettable(tlModule->L, -2); // * tucube clients client 
    lua_pushstring(tlModule->L, "request"); // * tucube clients client "request"
    lua_gettable(tlModule->L, -2); // * tucube clients client request
    lua_pushstring(tlModule->L, "headers"); // * tucube clients client request "headers"
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers
    lua_getglobal(tlModule->L, "string"); // * tucube clients client request headers string
    lua_pushstring(tlModule->L, "upper"); // * tucube clients client request headers string "upper"
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers string upper
    lua_pushstring(tlModule->L, headerField); // * tucube clients client request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube clients client request headers string errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube clients client request headers string errorString
        lua_pop(tlModule->L, 7); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * tucube clients client request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * tucube clients client request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tonumber(tlModule->L, -1);
    lua_pop(tlModule->L, 6); // *
    return 0;
}

int tucube_IHttp_onGetRequestStringHeader(struct tucube_Module* module, struct tucube_ClData* clData, const char* headerField, const char** headerValue) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "clients"); // * tucube "clients"
    lua_gettable(tlModule->L, -2); // * tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube clients clientId
    lua_gettable(tlModule->L, -2); // * tucube clients client 
    lua_pushstring(tlModule->L, "request"); // * tucube clients client "request"
    lua_gettable(tlModule->L, -2); // * tucube clients client request
    lua_pushstring(tlModule->L, "headers"); // * tucube clients client request "headers"
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers
    lua_getglobal(tlModule->L, "string"); // * tucube clients client request headers string
    lua_pushstring(tlModule->L, "upper"); // * tucube clients client request headers string "upper"
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers string upper
    lua_pushstring(tlModule->L, headerField); // * tucube clients client request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube clients client request headers string errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1)); // * tucube clients client request headers string errorString
        lua_pop(tlModule->L, 7); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * tucube clients client request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * tucube clients client request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * tucube clients client request headers headerValue
        warnx("%s: %u: Request header %s not found", __FILE__, __LINE__, headerField);
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tostring(tlModule->L, -1);
    lua_pop(tlModule->L, 6); // *
    return 0;
}

static int tucube_http_lua_onGetRequestContentLength(lua_State* L) {
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "contentLength"); // * client request "contentLength"
    lua_gettable(L, -2); // * client request contentLength
    lua_remove(L, -2); // * client contentLength
    return 1;
}

int tucube_IHttp_onGetRequestContentLength(struct tucube_Module* module, struct tucube_ClData* clData, ssize_t* contentLength) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // * tucube
    lua_pushstring(tlModule->L, "clients"); // * tucube "clients"
    lua_gettable(tlModule->L, -2); // * tucube clients
    lua_getglobal(tlModule->L, "onGetRequestContentLength"); // * tucube clients onGetRequestContentLength
    if(lua_isnil(tlModule->L, -1)) { // * tucube clients nil 
        lua_pop(tlModule->L, 3); //
        lua_pushcfunction(tlModule->L, tucube_http_lua_onGetRequestContentLength); // * tucube clients onGetRequestContentLength
    }
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // * tucube clients onGetRequestContentLength clientId
    lua_gettable(tlModule->L, -3); // * tucube clients onGetRequestContentLength client
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * tucube clients errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1));
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    if(lua_isnil(tlModule->L, -1)) { // * tucube clients nil 
        lua_pop(tlModule->L, 3); // *
        return -1;
    }
    *contentLength = lua_tointeger(tlModule->L, -1); // * tucube clients requestContentLength
    lua_pop(tlModule->L, 3); // *
    return 0;
}

int tucube_IHttp_onRequestFinish(struct tucube_Module* module, struct tucube_ClData* clData, void* args[]) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "tucube"); // tucube
    lua_pushstring(tlModule->L, "clients"); // tucube "clients"
    lua_gettable(tlModule->L, -2); // tucube clients
    lua_pushinteger(tlModule->L, GENC_CAST(clData->generic.pointer, struct tucube_http_lua_ClData*)->clientId); // tucube clients clientId
    lua_gettable(tlModule->L, -2); // tucube clients client
    lua_pushstring(tlModule->L, "request"); // tucube clients client "request"
    lua_gettable(tlModule->L, -2); // tucube clients client request
    const char* requestUri;
    const char* scriptPath;
    size_t scriptPathSize;
    const char* pathInfo;
    const char* queryString;
    lua_pushstring(tlModule->L, "requestUri"); // tucube clients client request "requestUri"
    lua_gettable(tlModule->L, -2); // tucube clients client request requestUri
    requestUri = lua_tostring(tlModule->L, -1);
    lua_pop(tlModule->L, 1); // tucube clients client request
    lua_pushstring(tlModule->L, "scriptPath"); // tucube clients client request "scriptPath"
    lua_gettable(tlModule->L, -2); // tucube clients client request scriptPath
    if(lua_isnil(tlModule->L, -1)) { // tucube clients client request nil
        lua_pop(tlModule->L, 1); // tucube clients client request
        lua_pushstring(tlModule->L, "scriptPath"); // tucube clients client request "scriptPath"
        scriptPath = "";
        scriptPathSize = sizeof("") - 1;
        lua_pushstring(tlModule->L, ""); // tucube clients client request "scriptPath" "" 
        lua_settable(tlModule->L, -3); // tucube clients client request
    } else { // tucube clients client request scriptPath
        scriptPath = lua_tolstring(tlModule->L, -1, &scriptPathSize);
        if(strncmp(scriptPath + scriptPathSize - 1, "/", sizeof("/") - 1) == 0) {
            warnx("%s: %u: scriptPath should not  end with /", __FILE__, __LINE__);
            lua_pop(tlModule->L, 5); //
            assert(lua_gettop(tlModule->L) == 0);
            return -1;
        }
        lua_pop(tlModule->L, 1); // tucube clients client request
    }
    if((pathInfo = strstr(requestUri, scriptPath)) != requestUri) { // if request uri doesn't begin with script name
        warnx("%s: %u: Request uri doesn't begin with script name", __FILE__, __LINE__);
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    pathInfo += scriptPathSize;
    if((queryString = strstr(pathInfo, "?")) != NULL) { // check if there is query string
        ++queryString; // query string begins after the question mark
        if(strstr(queryString, "?") != NULL) { // check if there is unnecessary question mark after query string
            warnx("%s: %u: Unnecessary question mark after query string", __FILE__, __LINE__);
            lua_pop(tlModule->L, 4); //
            assert(lua_gettop(tlModule->L) == 0);
            return -1;
        }
 
        lua_pushstring(tlModule->L, "pathInfo"); // tucube clients client request "pathInfo"
        if(queryString - pathInfo - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlModule->L, pathInfo, queryString - pathInfo - 1); // tucube clients client request "pathInfo" pathInfo
        else
            lua_pushstring(tlModule->L, "/"); // tucube clients client request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // tucube clients client request
	lua_pushstring(tlModule->L, "queryString"); // tucube clients client request "queryString"
        lua_pushstring(tlModule->L, queryString); // tucube clients client request "queryString" queryString 
        lua_settable(tlModule->L, -3); // tucube clients client request
    } else {
        lua_pushstring(tlModule->L, "queryString"); // tucube clients client request "queryString"
        lua_pushstring(tlModule->L, ""); // tucube clients client request "queryString" ""
        lua_settable(tlModule->L, -3); // tucube clients client request
        lua_pushstring(tlModule->L, "pathInfo"); // tucube clients client request "pathInfo"
        if(strlen(pathInfo) != 0) // check if path info is not empty string
            lua_pushstring(tlModule->L, pathInfo); // tucube clients client request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // tucube clients client request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // tucube clients client request
    }
    lua_pop(tlModule->L, 1); // tucube clients client 
    lua_getglobal(tlModule->L, "onRequestFinish"); // tucube clients client onRequestFinish
    if(lua_isnil(tlModule->L, -1)) { // tucube clients client nil
        warnx("%s: %u: onRequestFinish() is not found in the script", __FILE__, __LINE__);
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pushvalue(tlModule->L, -2); // tucube clients client onRequestFinish client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // tucube clients client errorString
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlModule->L, -1));
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 3); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_ICLocal_destroy(struct tucube_Module* module, struct tucube_ClData* clData) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    struct tucube_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    free(clData->generic.pointer);
    free(clData);
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int tucube_IBase_tlDestroy(struct tucube_Module* module) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
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

int tucube_IBase_destroy(struct tucube_Module* module) {
    warnx("%s: %u: %s", __FILE__, __LINE__, __FUNCTION__);
    pthread_key_delete(*module->tlModuleKey);
    free(module->tlModuleKey);
    free(module);
    return 0;
}
