#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <gaio.h>
#include <libgenc/genc_args.h>
#include <libgenc/genc_cast.h>
#include <libgenc/genc_List.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vinbero_common/vinbero_common_Log.h>
#include <vinbero_common/vinbero_common_Module.h>
#include <vinbero_common/vinbero_common_ClData.h>
#include <vinbero/vinbero_Interface_MODULE.h>
#include <vinbero/vinbero_Interface_CLOCAL.h>
#include <vinbero/vinbero_Interface_TLOCAL.h>
#include <vinbero/vinbero_Interface_HTTP.h>

struct vinbero_mt_http_lua_TlModule {
   lua_State* L;
};

struct vinbero_mt_http_lua_ClData {
    int clientId;
    struct vinbero_Interface_HTTP_Response* response;
    lua_State* L;
};

VINBERO_INTERFACE_MODULE_FUNCTIONS;
VINBERO_INTERFACE_TLOCAL_FUNCTIONS;
VINBERO_INTERFACE_CLOCAL_FUNCTIONS;
VINBERO_INTERFACE_HTTP_FUNCTIONS;

int vinbero_Interface_MODULE_init(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    vinbero_common_Module_init(module, "vinbero_mt_http_lua", "0.0.1", false);
    module->tlModuleKey = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlModuleKey, NULL);
    return 0;
}

int vinbero_Interface_MODULE_rInit(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    return 0;
}

static int vinbero_mt_http_lua_writeBytes(lua_State* L) {
    // * response bytes 
    char* bytes;
    size_t bytesSize;
    bytes = lua_tolstring(L, -1, &bytesSize); // * response bytess
    lua_pushstring(L, "cObject"); // * bytes string "cObject"
    lua_gettable(L, -3); // * response bytes cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response bytes cObject
    response->methods->writeBytes(response, bytes, bytesSize);
    lua_pop(L, 1); // * response bytes 
    return 0;
}

static int vinbero_mt_http_lua_writeIo(lua_State* L) {
    // * response file
    luaL_Stream* file = lua_touserdata(L, -1); // * response file 
    lua_pushstring(L, "cObject"); // * response file "cObject"
    lua_gettable(L, -3); // * response file cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response file cObject
    struct gaio_Io io;
    struct gaio_Methods ioMethods;
    GAIO_METHODS_NOP_INIT(&ioMethods);
    ioMethods.read = gaio_Fd_read;
    ioMethods.fileno = gaio_Fd_fileno;
    io.object.integer = fileno(file->f);
    io.methods = &ioMethods;
    struct stat statBuffer;
    fstat(io.object.integer, &statBuffer);
    response->methods->writeIo(response, &io, statBuffer.st_size);
    fclose(file->f);
    file->closef = NULL;
    lua_pop(L, 1); // * response file 
    return 0;
}

static int vinbero_mt_http_lua_writeCrLf(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response cObject
    response->methods->writeCrLf(response);
    lua_pop(L, 1); // * response
    return 0;
}

static int vinbero_mt_http_lua_writeVersion(lua_State* L) {
    // * response major minor
    int major = lua_tointeger(L, -2); // * response major minor
    int minor = lua_tointeger(L, -1); // * response major minor
    lua_pushstring(L, "cObject"); // * response major minor "cObject"
    lua_gettable(L, -4); // * response major minor cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response major minor cObject
    response->methods->writeVersion(response, major, minor);
    lua_pop(L, 1); // * response major minor
    return 0;
}

static int vinbero_mt_http_lua_writeStatusCode(lua_State* L) {
    // * response statusCode
    int statusCode = lua_tointeger(L, -1); // * response statusCode
    lua_pushstring(L, "cObject"); // * response statusCode "cObject"
    lua_gettable(L, -3); // * response statusCode cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response statusCode cObject
    response->methods->writeStatusCode(response, statusCode);
    lua_pop(L, 1); // * response statusCode
    return 0;
}

static int vinbero_mt_http_lua_writeIntHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    int headerValue = lua_tointeger(L, -1); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->methods->writeIntHeader(response, headerField, headerFieldSize, headerValue);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

static int vinbero_mt_http_lua_writeDoubleHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    double headerValue = lua_tonumber(L, -1); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->methods->writeDoubleHeader(response, headerField, headerFieldSize, headerValue);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

static int vinbero_mt_http_lua_writeStringHeader(lua_State* L) {
    // * response headerField headerValue
    char* headerField;
    size_t headerFieldSize;
    headerField = lua_tolstring(L, -2, &headerFieldSize); // * response headerField headerValue
    char* headerValue;
    size_t headerValueSize;
    headerValue = lua_tolstring(L, -1, &headerValueSize); // * response headerField headerValue
    lua_pushstring(L, "cObject"); // * response headerField headerValue "cObject"
    lua_gettable(L, -4); // * response headerField headerValue cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response headerField headerValue cObject
    response->methods->writeStringHeader(response, headerField, headerFieldSize, headerValue, headerValueSize);
    lua_pop(L, 1); // * response headerField headerValue
    return 0;
}

static int vinbero_mt_http_lua_writeStringBody(lua_State* L) {
    // * response stringBody
    char* stringBody;
    size_t stringBodySize;
    stringBody = lua_tolstring(L, -1, &stringBodySize);
    lua_pushstring(L, "cObject"); // * response stringBody "cObject"
    lua_gettable(L, -3); // * response stringBody cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeStringBody(response, stringBody, stringBodySize);
    return 0;
}

static int vinbero_mt_http_lua_writeIoBody(lua_State* L) {
    // * response fileBody
    luaL_Stream* fileBody = lua_touserdata(L, -1);
    lua_pushstring(L, "cObject"); // * response fileBody "cObject"
    lua_gettable(L, -3); // * response fileBody cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1); // * response fileBody cObject
    struct gaio_Io io;
    struct gaio_Methods ioMethods;
    GAIO_METHODS_NOP_INIT(&ioMethods);
    ioMethods.read = gaio_Fd_read;
    ioMethods.fileno = gaio_Fd_fileno;
    io.object.integer = fileno(fileBody->f);
    io.methods = &ioMethods;
    struct stat statBuffer;
    fstat(io.object.integer, &statBuffer);
    response->methods->writeIoBody(response, &io, statBuffer.st_size);
    fclose(fileBody->f);
    fileBody->closef = NULL;
    lua_pop(L, 1); // * response fileBody
    return 0;
}

static int vinbero_mt_http_lua_writeChunkedBodyStart(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeChunkedBodyStart(response);
    return 0;
}

static int vinbero_mt_http_lua_writeChunkedBody(lua_State* L) {
    // * response chunkedBody
    char* chunkedBody;
    size_t chunkedBodySize;
    chunkedBody = lua_tolstring(L, -1, &chunkedBodySize);
    lua_pushstring(L, "cObject"); // * response chunkedBody "cObject"
    lua_gettable(L, -3); // * response chunkedBody cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeChunkedBody(response, chunkedBody, chunkedBodySize);
    return 0;
}

static int vinbero_mt_http_lua_writeChunkedBodyEnd(lua_State* L) {
    // * response
    lua_pushstring(L, "cObject"); // * response "cObject"
    lua_gettable(L, -2); // * response cObject
    struct vinbero_Interface_HTTP_Response* response = lua_touserdata(L, -1);
    response->methods->writeChunkedBodyEnd(response);
    return 0;
}

int vinbero_Interface_TLOCAL_init(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    const char* scriptFile;
    int ret;
    if((ret = vinbero_common_Config_getRequiredString(module->config, module, "vinbero_mt_http_lua.scriptFile", &scriptFile))) {
        VINBERO_COMMON_LOG_ERROR("vinbero_mt_http_lua.scriptFile is required");
        return ret;
    }
    struct vinbero_mt_http_lua_TlModule* tlModule = malloc(sizeof(struct vinbero_mt_http_lua_TlModule));
    tlModule->L = luaL_newstate();
    luaL_openlibs(tlModule->L);
    lua_newtable(tlModule->L); // vinbero
    lua_pushstring(tlModule->L, "args"); // vinbero "args"
    char* scriptArgs;
    vinbero_common_Config_getString(module->config, module, "vinbero_mt_http_lua.scriptArgs", &scriptArgs, NULL);
    if(scriptArgs != NULL)
        lua_pushstring(tlModule->L, scriptArgs); // vinbero "args" args
    else
        lua_pushnil(tlModule->L); // vinbero "args" nil
    lua_settable(tlModule->L, -3); // vinbero 
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_newtable(tlModule->L); // vinbero "clients" clients
    lua_settable(tlModule->L, -3); // vinbero
    lua_pushstring(tlModule->L, "responseMethods"); // vinbero "responseMethods"
    lua_newtable(tlModule->L); // vinbero "responseMethods" reponseMethods
    lua_pushstring(tlModule->L, "__index"); // vinbero "responseMethods" responseMethods "__index"
    lua_pushvalue(tlModule->L, -2); // vinbero "responseMethods" responseMethods "__index" responseMethods
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeBytes"); // vinbero "responseMethods" responseMethods "writeBytes"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeBytes); // vinbero "responseMethods" responseMethods "writeBytes" writeBytes
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeIo"); // vinbero "responseMethods" responseMethods "writeIo"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeIo); // vinbero "responseMethods" responseMethods "writeIo" writeIo
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeCrLf"); // vinbero "responseMethods" responseMethods "writeCrLf"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeCrLf); // vinbero "responseMethods" responseMethods "writeCrLf" writeCrLf"
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods 
    lua_pushstring(tlModule->L, "writeVersion"); // vinbero "responseMethods" responseMethods "writeVersion"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeVersion); // vinbero "responseMethods" responseMethods "writeVersion" writeVersion
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeStatusCode"); // vinbero "responseMethods" responseMethods "writeStatusCode"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeStatusCode); // vinbero "responseMethods" responseMethods "writeStatusCode" writeStatusCode
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeIntHeader"); // vinbero "responseMethods" responseMethods "writeIntHeader"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeIntHeader); // vinbero "responseMethods" responseMethods "writeIntHeader" writeIntHeader
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeDoubleHeader"); // vinbero "responseMethods" responseMethods "writeDoubleHeader"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeDoubleHeader); // vinbero "responseMethods" responseMethods "writeDoubleHeader" writeDoubleHeader
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeStringHeader"); // vinbero "responseMethods" responseMethods "writeStringHeader" writeStringHeader
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeStringHeader); // vinbero "responseMethods" responseMethods "writeStringHeader" writeStringHeader
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeStringBody"); // vinbero "responseMethods" responseMethods "writeStringBody"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeStringBody); // vinbero "responseMethods" responseMethods "writeStringBody" writeStringBody
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeIoBody"); // vinbero "responseMethods" responseMethods "writeIoBody"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeIoBody); // vinbero "responseMethods" responseMethods "writeIoBody" writeIoBody
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeChunkedBodyStart"); // vinbero "responseMethods" responseMethods "writeChunkedBodyStart"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeChunkedBodyStart); // vinbero "responseMethods" responseMethods "writeChunkedBodyStart" writeChunkedBodyStart
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeChunkedBody"); // vinbero "responseMethods" responseMethods "writeChunkedBody"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeChunkedBody); // vinbero "responseMethods" responseMethods "writeChunkedBody" writeChunkedBody
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods
    lua_pushstring(tlModule->L, "writeChunkedBodyEnd"); // vinbero "responseMethods" responseMethods "writeChunkedBodyEnd"
    lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_writeChunkedBodyEnd); // vinbero "responseMethods" responseMethods "writeChunkedBodyEnd" writeChunkedBodyEnd
    lua_settable(tlModule->L, -3); // vinbero "responseMethods" responseMethods

    lua_settable(tlModule->L, -3); // vinbero
    lua_setglobal(tlModule->L, "vinbero"); //
    if(luaL_loadfile(tlModule->L, scriptFile) != LUA_OK) { // chunk
        VINBERO_COMMON_LOG_ERROR("luaL_loadfile() failed");
        pthread_exit(NULL);
    }
    if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { // errorString 
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // errorString
        lua_pop(tlModule->L, 1); //
        pthread_exit(NULL);
    }
    lua_getglobal(tlModule->L, "onInit"); // onInit
    if(lua_isnil(tlModule->L, -1)) // nil
        lua_pop(tlModule->L, 1); //
    else { // onInit
        if(lua_pcall(tlModule->L, 0, 0, 0) != 0) { //
            VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // errorString
            lua_pop(tlModule->L, 1); //
	    pthread_exit(NULL);
        }
    }
    assert(lua_gettop(tlModule->L) == 0);
    pthread_setspecific(*module->tlModuleKey, tlModule);
    return 0;
}

int vinbero_Interface_TLOCAL_rInit(struct vinbero_common_Module* module) {
    return 0;
}

int vinbero_Interface_CLOCAL_init(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 1) return -1;
    clData->generic.pointer = malloc(1 * sizeof(struct vinbero_mt_http_lua_ClData));
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    localClData->clientId = gaio_Fd_fileno(GENC_CAST(args[0], struct vinbero_Interface_HTTP_Response*)->io);
    localClData->response = args[0];
    return 0;
}

int vinbero_Interface_HTTP_onRequestStart(void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_newtable(tlModule->L); // vinbero clients clientId client
    lua_pushstring(tlModule->L, "request"); // vinbero clients clientId client "request"
    lua_newtable(tlModule->L); // vinbero clients clientId client "request" request
    lua_pushstring(tlModule->L, "headers"); // vinbero clients clientId client "request" request "headers"
    lua_newtable(tlModule->L); // vinbero clients clientId client "request" request "headers" headers
    lua_settable(tlModule->L, -3); // vinbero clients clientId client "request" request
    lua_pushstring(tlModule->L, "body"); // vinbero clients clientId client "request" request "body"
    lua_pushstring(tlModule->L, ""); // vinbero clients clientId client "request" request "body" ""
    lua_settable(tlModule->L, -3); // vinbero clients clientId client "request" request
    lua_pushstring(tlModule->L, "contentLength"); // vinbero clients clientId client "request" request "contentLength"
    lua_pushinteger(tlModule->L, 0); // vinbero clients clientId client "request" request "contentLength" 0
    lua_settable(tlModule->L, -3); // vinbero clients clientId client "request" request
    lua_settable(tlModule->L, -3); // vinbero clients clientId client
    lua_pushstring(tlModule->L, "response"); // vinbero clients clientId client "response"
    lua_newtable(tlModule->L); // vinbero clients clientId client "response" response
    lua_pushstring(tlModule->L, "cObject"); // vinbero clients clientId client "response" response "cObject"
    lua_pushlightuserdata(tlModule->L, localClData->response); // vinbero clients clientId client "response" response "cObject" cObject
    lua_settable(tlModule->L, -3); // vinbero clients clientId client "response" response
    lua_pushstring(tlModule->L, "responseMethods"); // vinbero clients clientId client "response" response "responseMethods"
    lua_gettable(tlModule->L, -7); // vinbero clients clientId client "response" response responseMethods
    lua_setmetatable(tlModule->L, -2); // vinbero clients clientId client "response" response
    lua_settable(tlModule->L, -3); // vinbero clients clientId client
    lua_settable(tlModule->L, -3); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_remove(tlModule->L, -2); // vinbero client
    lua_getglobal(tlModule->L, "onRequestStart"); // vinbero client onRequestStart
    if(lua_isnil(tlModule->L, -1)) { // vinbero client nil
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return 0;
    }
    lua_pushvalue(tlModule->L, -2); // vinbero client onRequestStart client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // vinbero client errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // vinbero client errorString
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestMethod(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "method"); // vinbero clients client request "method"
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request "method" method 
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); // 
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestUri(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "requestUri"); // vinbero clients client request "requestUri"
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request "requestUri" requestUri 
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestVersionMajor(int major, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "versionMajor"); // vinbero clients client request "versionMajor"
    lua_pushinteger(tlModule->L, major); // vinbero clients client request "versionMajor" versionMajor 
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestVersionMinor(int minor, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "versionMinor"); // vinbero clients client request "versionMinor"
    lua_pushinteger(tlModule->L, minor); // vinbero clients client request "versionMinor" versionMinor
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestContentLength(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "contentLength"); // vinbero clients client request "contentLength"
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request "contentLength" contentLength
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); // 
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestContentType(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "contentType"); // vinbero clients client request "contentType"
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request "contentType" contentType
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestScriptPath(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clientss clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client 
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "scriptPath"); // vinbero clients client request "scriptPath"
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request "scriptPath" scriptPath
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestHeaderField(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "recentHeaderField"); // vinbero clients client request "recentHeaderField"
    for(ssize_t index = 0; index < tokenSize; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request "recentHeaderField" recentHeaderField
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestHeaderValue(char* token, ssize_t tokenSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    lua_pushstring(tlModule->L, "recentHeaderField"); // vinbero clients client request "recentHeaderField"
    lua_gettable(tlModule->L, -2); // vinbero clients client request recentHeaderField
    lua_pushvalue(tlModule->L, -2); // vinbero clients client request recentHeaderField request
    lua_pushstring(tlModule->L, "headers"); // vinbero clients client request recentHeaderField request "headers"
    lua_gettable(tlModule->L, -2); // vinbero clients client request recentHeaderField request headers
    lua_pushvalue(tlModule->L, -3); // vinbero clients client request recentHeaderField request headers requestHeaderField
    lua_pushlstring(tlModule->L, token, tokenSize); // vinbero clients client request recentHeaderField request headers recentHeaderField token
    lua_settable(tlModule->L, -3); // vinbero clients client request recentHeaderField request headers
    lua_pop(tlModule->L, 3); // vinbero clients client request
    lua_pushstring(tlModule->L, "recentHeaderField"); // vinbero clients client request "recentHeaderField"
    lua_pushnil(tlModule->L); // vinbero clients client request "recentHeaderField" nil
    lua_settable(tlModule->L, -3); // vinbero clients client request
    lua_pop(tlModule->L, 4); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onRequestHeadersFinish(void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_getglobal(tlModule->L, "onRequestHeadersFinish"); // vinbero clients client onRequestHeadersFinish
    if(lua_isnil(tlModule->L, -1)) { // vinbero clients client nil
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return 0;
    }
    lua_pushvalue(tlModule->L, -2); // vinbero clients client onRequestHeadersFinish client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // vinbero clients client errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1));
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 1); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

static int vinbero_mt_http_lua_onRequestBodyStart(lua_State* L) {
    VINBERO_COMMON_LOG_TRACE2();
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "body"); // * client request "body"
    lua_newtable(L); // * client request "body" body
    lua_settable(L, -3); // * client request
    lua_pop(L, 1); // * client
    return 0;
}

int vinbero_Interface_HTTP_onRequestBodyStart(void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_getglobal(tlModule->L, "onRequestBodyStart"); // vinbero clients client onRequestBodyStart
    if(lua_isnil(tlModule->L, -1)) { // vinbero clients client nil
        lua_pop(tlModule->L, 1); // vinbero clients client
        lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_onRequestBodyStart); // vinbero clients client onRequestBodyStart
    }
    lua_pushvalue(tlModule->L, -2); // vinbero clients client onRequestBodyStart client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // vinbero clients client errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // vinbero clients client errorString
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
	return -1;
    }
    lua_pop(tlModule->L, 3); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

static int vinbero_mt_http_lua_onRequestBody(lua_State* L) {
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
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(L, -1));
        lua_pop(L, 3); // * client bodyChunk
        luaL_error(L, "vinbero_mt_http_lua_onRequestBody() failed"); //
    }
    lua_pop(L, 2); // * client bodyChunk
    return 0;
}

int vinbero_Interface_HTTP_onRequestBody(char* bodyChunk, ssize_t bodyChunkSize, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_getglobal(tlModule->L, "onRequestBody"); // vinbero clients onRequestBody
    if(lua_isnil(tlModule->L, -1)) { // vinbero clients nil
        lua_pop(tlModule->L, 1); //  vinbero clients
        lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_onRequestBody); // vinbero clients onRequestBody
    }
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients onRequestBody clientId
    lua_gettable(tlModule->L, -3); // vinbero clients onRequestBody client
    lua_pushlstring(tlModule->L, bodyChunk, bodyChunkSize); // vinbero clients onRequestBody client bodyChunk
    if(lua_pcall(tlModule->L, 2, 0, 0) != 0) { // vinbero clients errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // vinbero clients errorString
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}
static int vinbero_mt_http_lua_onRequestBodyFinish(lua_State* L) {
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
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(L, -1)); // * client request body table errorString
        lua_pop(L, 4); // * client
	luaL_error(L, "vinbero_mt_http_lua_onRequestBodyFinish() failed");
    }
    lua_pushvalue(L, -4); // * client request body table result request
    lua_pushstring(L, "body"); // * client request body table result request "body"
    lua_pushvalue(L, -3); // * client request body table result request "body" result
    lua_settable(L, -3); // * client request body table result request
    lua_pop(L, 5); // * client
    return 0;
}

int vinbero_Interface_HTTP_onRequestBodyFinish(void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    int argc;
    GENC_ARGS_COUNT(args, &argc);
    if(argc != 2) return -1;
    struct vinbero_common_Module* module = args[0];
    struct vinbero_common_ClData* clData = args[1];
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_getglobal(tlModule->L, "onRequestBodyFinish"); // vinbero clients onRequestBodyFinish
    if(lua_isnil(tlModule->L, -1)) { // vinbero clients nil
        lua_pop(tlModule->L, 1); // vinbero clients
        lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_onRequestBodyFinish); // vinbero clients onRequestBodyFinish
    } 
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients onRequestBodyFinish clientId
    lua_gettable(tlModule->L, -3); // vinbero clients onRequestBodyFinish client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // vinbero clients errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // vinbero clients errorString
        lua_pop(tlModule->L, 3); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 2); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_HTTP_onGetRequestIntHeader(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, const char* headerField, int* headerValue) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    lua_getglobal(tlModule->L, "vinbero"); // * vinbero
    lua_pushstring(tlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(tlModule->L, -2); // * vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // * vinbero clients clientId
    lua_gettable(tlModule->L, -2); // * vinbero clients client 
    lua_pushstring(tlModule->L, "request"); // * vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request
    lua_pushstring(tlModule->L, "headers"); // * vinbero clients client request "headers"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers
    lua_getglobal(tlModule->L, "string"); // * vinbero clients client request headers string
    lua_pushstring(tlModule->L, "upper"); // * vinbero clients client request headers string "upper"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers string upper
    lua_pushstring(tlModule->L, headerField); // * vinbero clients client request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * vinbero clients client request headers string errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // * vinbero clients client request headers string errorString
        lua_pop(tlModule->L, 7); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * vinbero clients client request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * vinbero clients client request headers headerValue
        VINBERO_COMMON_LOG_ERROR("Request header %s not found", headerField);
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tointeger(tlModule->L, -1);
    lua_pop(tlModule->L, 6); // *
    return 0;
}
int vinbero_Interface_HTTP_onGetRequestDoubleHeader(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, const char* headerField, double* headerValue) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    lua_getglobal(tlModule->L, "vinbero"); // * vinbero
    lua_pushstring(tlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(tlModule->L, -2); // * vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // * vinbero clients clientId
    lua_gettable(tlModule->L, -2); // * vinbero clients client 
    lua_pushstring(tlModule->L, "request"); // * vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request
    lua_pushstring(tlModule->L, "headers"); // * vinbero clients client request "headers"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers
    lua_getglobal(tlModule->L, "string"); // * vinbero clients client request headers string
    lua_pushstring(tlModule->L, "upper"); // * vinbero clients client request headers string "upper"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers string upper
    lua_pushstring(tlModule->L, headerField); // * vinbero clients client request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * vinbero clients client request headers string errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // * vinbero clients client request headers string errorString
        lua_pop(tlModule->L, 7); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * vinbero clients client request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * vinbero clients client request headers headerValue
        VINBERO_COMMON_LOG_ERROR("Request header %s not found", headerField);
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tonumber(tlModule->L, -1);
    lua_pop(tlModule->L, 6); // *
    return 0;
}

int vinbero_Interface_HTTP_onGetRequestStringHeader(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, const char* headerField, const char** headerValue) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    lua_getglobal(tlModule->L, "vinbero"); // * vinbero
    lua_pushstring(tlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(tlModule->L, -2); // * vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // * vinbero clients clientId
    lua_gettable(tlModule->L, -2); // * vinbero clients client 
    lua_pushstring(tlModule->L, "request"); // * vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request
    lua_pushstring(tlModule->L, "headers"); // * vinbero clients client request "headers"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers
    lua_getglobal(tlModule->L, "string"); // * vinbero clients client request headers string
    lua_pushstring(tlModule->L, "upper"); // * vinbero clients client request headers string "upper"
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers string upper
    lua_pushstring(tlModule->L, headerField); // * vinbero clients client request headers string upper headerField
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * vinbero clients client request headers string errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); // * vinbero clients client request headers string errorString
        lua_pop(tlModule->L, 7); // *
        return -1;
    }
    lua_remove(tlModule->L, -2); // * vinbero clients client request headers upperedHeaderField
    lua_gettable(tlModule->L, -2); // * vinbero clients client request headers headerValue
    if(lua_isnil(tlModule->L, -1)) { // * vinbero clients client request headers headerValue
        VINBERO_COMMON_LOG_WARN("Request header %s not found", headerField);
        lua_pop(tlModule->L, 6); // *
        return -1;
    }
    *headerValue = lua_tostring(tlModule->L, -1);
    lua_pop(tlModule->L, 6); // *
    return 0;
}

static int vinbero_mt_http_lua_onGetRequestContentLength(lua_State* L) {
    // * client
    lua_pushstring(L, "request"); // * client "request"
    lua_gettable(L, -2); // * client request
    lua_pushstring(L, "contentLength"); // * client request "contentLength"
    lua_gettable(L, -2); // * client request contentLength
    lua_remove(L, -2); // * client contentLength
    return 1;
}

int vinbero_Interface_HTTP_onGetRequestContentLength(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, ssize_t* contentLength) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    lua_getglobal(tlModule->L, "vinbero"); // * vinbero
    lua_pushstring(tlModule->L, "clients"); // * vinbero "clients"
    lua_gettable(tlModule->L, -2); // * vinbero clients
    lua_getglobal(tlModule->L, "onGetRequestContentLength"); // * vinbero clients onGetRequestContentLength
    if(lua_isnil(tlModule->L, -1)) { // * vinbero clients nil 
        lua_pop(tlModule->L, 3); //
        lua_pushcfunction(tlModule->L, vinbero_mt_http_lua_onGetRequestContentLength); // * vinbero clients onGetRequestContentLength
    }
    lua_pushinteger(tlModule->L, localClData->clientId); // * vinbero clients onGetRequestContentLength clientId
    lua_gettable(tlModule->L, -3); // * vinbero clients onGetRequestContentLength client
    if(lua_pcall(tlModule->L, 1, 1, 0) != 0) { // * vinbero clients errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1));
        lua_pop(tlModule->L, 3); //
        return -1;
    }
    if(lua_isnil(tlModule->L, -1)) { // * vinbero clients nil 
        lua_pop(tlModule->L, 3); // *
        return -1;
    }
    *contentLength = lua_tointeger(tlModule->L, -1); // * vinbero clients requestContentLength
    lua_pop(tlModule->L, 3); // *
    return 0;
}

int vinbero_Interface_HTTP_onRequestFinish(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData, void* args[]) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    struct vinbero_mt_http_lua_ClData* localClData = clData->generic.pointer;
    lua_getglobal(tlModule->L, "vinbero"); // vinbero
    lua_pushstring(tlModule->L, "clients"); // vinbero "clients"
    lua_gettable(tlModule->L, -2); // vinbero clients
    lua_pushinteger(tlModule->L, localClData->clientId); // vinbero clients clientId
    lua_gettable(tlModule->L, -2); // vinbero clients client
    lua_pushstring(tlModule->L, "request"); // vinbero clients client "request"
    lua_gettable(tlModule->L, -2); // vinbero clients client request
    const char* requestUri;
    const char* scriptPath;
    size_t scriptPathSize;
    const char* pathInfo;
    const char* queryString;
    lua_pushstring(tlModule->L, "requestUri"); // vinbero clients client request "requestUri"
    lua_gettable(tlModule->L, -2); // vinbero clients client request requestUri
    requestUri = lua_tostring(tlModule->L, -1);
    lua_pop(tlModule->L, 1); // vinbero clients client request
    lua_pushstring(tlModule->L, "scriptPath"); // vinbero clients client request "scriptPath"
    lua_gettable(tlModule->L, -2); // vinbero clients client request scriptPath
    if(lua_isnil(tlModule->L, -1)) { // vinbero clients client request nil
        lua_pop(tlModule->L, 1); // vinbero clients client request
        lua_pushstring(tlModule->L, "scriptPath"); // vinbero clients client request "scriptPath"
        scriptPath = "";
        scriptPathSize = sizeof("") - 1;
        lua_pushstring(tlModule->L, ""); // vinbero clients client request "scriptPath" "" 
        lua_settable(tlModule->L, -3); // vinbero clients client request
    } else { // vinbero clients client request scriptPath
        scriptPath = lua_tolstring(tlModule->L, -1, &scriptPathSize);
        if(strncmp(scriptPath + scriptPathSize - 1, "/", sizeof("/") - 1) == 0) {
            VINBERO_COMMON_LOG_ERROR("scriptPath should not  end with /");
            lua_pop(tlModule->L, 5); //
            assert(lua_gettop(tlModule->L) == 0);
            return -1;
        }
        lua_pop(tlModule->L, 1); // vinbero clients client request
    }
    if((pathInfo = strstr(requestUri, scriptPath)) != requestUri) { // if request uri doesn't begin with script name
        VINBERO_COMMON_LOG_ERROR("Request uri doesn't begin with script name");
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    pathInfo += scriptPathSize;
    if((queryString = strstr(pathInfo, "?")) != NULL) { // check if there is query string
        ++queryString; // query string begins after the question mark
        if(strstr(queryString, "?") != NULL) { // check if there is unnecessary question mark after query string
            VINBERO_COMMON_LOG_ERROR("Unnecessary question mark after query string");
            lua_pop(tlModule->L, 4); //
            assert(lua_gettop(tlModule->L) == 0);
            return -1;
        }
 
        lua_pushstring(tlModule->L, "pathInfo"); // vinbero clients client request "pathInfo"
        if(queryString - pathInfo - 1 != 0) // check if path info is not empty string
            lua_pushlstring(tlModule->L, pathInfo, queryString - pathInfo - 1); // vinbero clients client request "pathInfo" pathInfo
        else
            lua_pushstring(tlModule->L, "/"); // vinbero clients client request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // vinbero clients client request
	lua_pushstring(tlModule->L, "queryString"); // vinbero clients client request "queryString"
        lua_pushstring(tlModule->L, queryString); // vinbero clients client request "queryString" queryString 
        lua_settable(tlModule->L, -3); // vinbero clients client request
    } else {
        lua_pushstring(tlModule->L, "queryString"); // vinbero clients client request "queryString"
        lua_pushstring(tlModule->L, ""); // vinbero clients client request "queryString" ""
        lua_settable(tlModule->L, -3); // vinbero clients client request
        lua_pushstring(tlModule->L, "pathInfo"); // vinbero clients client request "pathInfo"
        if(strlen(pathInfo) != 0) // check if path info is not empty string
            lua_pushstring(tlModule->L, pathInfo); // vinbero clients client request "pathInfo" pathInfo 
        else
            lua_pushstring(tlModule->L, "/"); // vinbero clients client request "pathInfo" pathInfo 
        lua_settable(tlModule->L, -3); // vinbero clients client request
    }
    lua_pop(tlModule->L, 1); // vinbero clients client 
    lua_getglobal(tlModule->L, "onRequestFinish"); // vinbero clients client onRequestFinish
    if(lua_isnil(tlModule->L, -1)) { // vinbero clients client nil
        VINBERO_COMMON_LOG_ERROR("onRequestFinish() is not found in the script");
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pushvalue(tlModule->L, -2); // vinbero clients client onRequestFinish client
    if(lua_pcall(tlModule->L, 1, 0, 0) != 0) { // vinbero clients client errorString
        VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1));
        lua_pop(tlModule->L, 4); //
        assert(lua_gettop(tlModule->L) == 0);
        return -1;
    }
    lua_pop(tlModule->L, 3); //
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_CLOCAL_destroy(struct vinbero_common_Module* module, struct vinbero_common_ClData* clData) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    free(clData->generic.pointer);
    free(clData);
    assert(lua_gettop(tlModule->L) == 0);
    return 0;
}

int vinbero_Interface_TLOCAL_destroy(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    return 0;
}

int vinbero_Interface_TLOCAL_rDestroy(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    struct vinbero_mt_http_lua_TlModule* tlModule = pthread_getspecific(*module->tlModuleKey);
    if(tlModule != NULL) {
        lua_getglobal(tlModule->L, "onDestroy"); // onDestroy
        if(lua_isnil(tlModule->L, -1))
            lua_pop(tlModule->L, 1); //
        else {
            if(lua_pcall(tlModule->L, 0, 0, 0) != 0) {
                VINBERO_COMMON_LOG_ERROR("%s", lua_tostring(tlModule->L, -1)); //errorString
                lua_pop(tlModule->L, 1); //
            }
        }
        lua_close(tlModule->L);
        free(tlModule);
    }
    return 0;
}

int vinbero_Interface_MODULE_destroy(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    return 0;
}

int vinbero_Interface_MODULE_rDestroy(struct vinbero_common_Module* module) {
    VINBERO_COMMON_LOG_TRACE2();
    pthread_key_delete(*module->tlModuleKey);
    free(module->tlModuleKey);
    free(module);
    return 0;
}
