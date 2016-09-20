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

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list)
{
    struct tucube_module* module = malloc(1 * sizeof(struct tucube_module));
    GONC_LIST_ELEMENT_INIT(module);
    module->tlmodule_key = malloc(1 * sizeof(pthread_key_t));
    pthread_key_create(module->tlmodule_key, NULL);

    GONC_LIST_APPEND(module_list, module);

    return 0;
}

int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args)
{
    char* script_file = NULL;
    GONC_LIST_FOR_EACH(module_args, struct tucube_module_arg, module_arg)
    {
        if(strncmp("script-file", module_arg->name, sizeof("script-file")) == 0)
	     script_file = module_arg->value;
    }

    if(script_file == NULL)
    {
        warnx("%s: %u: Argument script-file is required", __FILE__, __LINE__);
        pthread_exit(NULL);
    }

    struct tucube_http_lua_tlmodule* tlmodule = malloc(sizeof(struct tucube_http_lua_tlmodule));

    tlmodule->L = luaL_newstate();
    luaL_openlibs(tlmodule->L);

    lua_newtable(tlmodule->L); // requests
    lua_setglobal(tlmodule->L, "requests"); //

    if(luaL_loadfile(tlmodule->L, script_file) != LUA_OK) // file
    {
        warnx("%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    if(lua_pcall(tlmodule->L, 0, 0, 0) != 0) //
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // error_string
        lua_pop(tlmodule->L, 1); //
        pthread_exit(NULL);
    }

    lua_getglobal(tlmodule->L, "onInit"); // onInit
    if(lua_isnil(tlmodule->L, -1))
        lua_pop(tlmodule->L, 1); //
    else
    {
        if(lua_pcall(tlmodule->L, 0, 0, 0) != 0)
        {
            warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); //error_string
            lua_pop(tlmodule->L, 1); //
        }
    }

    pthread_setspecific(*module->tlmodule_key, tlmodule);

    return 0;
}

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_cldata_list* cldata_list, int* client_socket)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    struct tucube_cldata* cldata = malloc(sizeof(struct tucube_cldata));
    GONC_LIST_ELEMENT_INIT(cldata);
    cldata->pointer = malloc(1 * sizeof(struct tucube_http_lua_cldata));
    GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket = client_socket;
    GONC_LIST_APPEND(cldata_list, cldata);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *client_socket); // requests client_socket
    lua_newtable(tlmodule->L); // requests client_socket request
    lua_pushstring(tlmodule->L, "body"); // requests client_socket request "body"
    lua_pushstring(tlmodule->L, ""); // requests client_socket request "body" ""
    lua_settable(tlmodule->L, -3); // requests client_socket request
    lua_pushstring(tlmodule->L, "headers"); // requests client_socket request "headers"
    lua_newtable(tlmodule->L); // requests client_socket request "headers" headers
    lua_settable(tlmodule->L, -3); // requests client_socket request
    lua_pushstring(tlmodule->L, "parameters"); // requests client_socket request "parameters"
    lua_newtable(tlmodule->L); // requests client_socket request "parameters" parameters
    lua_settable(tlmodule->L, -3); // requests client_socket request
    lua_settable(tlmodule->L, -3); // requests
    lua_pop(tlmodule->L, 1); //

    return 0;
}


int tucube_epoll_http_module_on_request_start(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onRequestStart"); // onRequestStart
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1); //
        return 0;
    }
    lua_getglobal(tlmodule->L, "requests"); // onRequestStart requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // onRequestStart requests client_socket
    lua_gettable(tlmodule->L, -2); // onRequestStart requests request
    lua_remove(tlmodule->L, -2); // onRequestStart request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) //
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // error_string
        lua_pop(tlmodule->L, 1);
        return -1;
    }
    return 0;
}

int tucube_epoll_http_module_on_method(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "method"); // requests request "method"
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "method" method 
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 

    return 0;
}

int tucube_epoll_http_module_on_request_uri(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "requestURI"); // requests request "requestURI"
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "requestURI" requestURI 
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_on_protocol(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "protocol"); // requests request "protocol"
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "protocol" protocol 
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_on_content_length(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); //requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "contentLength"); // requests request "contentLength"
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "contentLength" contentLength
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 
    return 0;
}

int tucube_epoll_http_module_on_content_type(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); //requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "contentType"); // requests request "contentType"
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "contentType" contentType
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 
    return 0;
}

int tucube_epoll_http_module_on_script_path(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); //requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "scriptPath"); // requests request "scriptPath"
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "scriptPath" scriptPath
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); // 
    return 0;
}

int tucube_epoll_http_module_on_header_field(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); //requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "recentHeaderField"); // requests request "recentHeaderField"

    for(ssize_t index = 0; index < token_size; ++index)
        token[index] = toupper(token[index]);
    lua_pushlstring(tlmodule->L, token, token_size); // requests request "recentHeaderField" recentHeaderField
    lua_settable(tlmodule->L, -3); //requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_on_header_value(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // requests client_socket
    lua_gettable(tlmodule->L, -2); // requests request
    lua_pushstring(tlmodule->L, "recentHeaderField"); // requests request "recentHeaderField"
    lua_gettable(tlmodule->L, -2); // requests request recentHeaderField
    lua_pushvalue(tlmodule->L, -2); // requests request recentHeaderField request
    lua_pushstring(tlmodule->L, "headers"); // requests request recentHeaderField request "headers"
    lua_gettable(tlmodule->L, -2); // requests request recentHeaderField request headers
    lua_pushvalue(tlmodule->L, -3); // requests request recentHeaderField request headers requestHeaderField
    lua_pushlstring(tlmodule->L, token, token_size); // requests request recentHeaderField request headers recentHeaderField token
    lua_settable(tlmodule->L, -3); // requests request recentHeaderField request headers
    lua_pop(tlmodule->L, 3); // requests request
    lua_pushstring(tlmodule->L, "recentHeaderField"); // requests request "recentHeaderField"
    lua_pushnil(tlmodule->L); // requests request "recentHeaderField" nil
    lua_settable(tlmodule->L, -3); // requests request
    lua_pop(tlmodule->L, 2); //

    return 0;
}

int tucube_epoll_http_module_on_headers_finish(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "onHeadersFinish"); // onHeadersFinish
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1); //
        return 0;
    }
    lua_getglobal(tlmodule->L, "requests"); // onHeadersFinish requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // onHeadersFinish requests client_socket
    lua_gettable(tlmodule->L, -2); // onHeadersFinish requests request
    lua_remove(tlmodule->L, -2); // onHeadersFinish request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) //
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // error_string
        lua_pop(tlmodule->L, 1); //
        return -1;
    }

    return 0;
}

int tucube_epoll_http_module_get_content_length(struct tucube_module* module, struct tucube_cldata* cldata, ssize_t* content_length)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    lua_getglobal(tlmodule->L, "getContentLength"); // getContentLength
    if(lua_isnil(tlmodule->L, -1))
    {
        *content_length = 0;
        lua_pop(tlmodule->L, 1); //
        return 0;
    }

    lua_getglobal(tlmodule->L, "requests"); // getContentLength requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // getContentLength requests client_socket
    lua_gettable(tlmodule->L, -2); // getContentLength requests request
    lua_remove(tlmodule->L, -2); // getContentLength request
    if(lua_pcall(tlmodule->L, 1, 1, 0) != 0) // content_length
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // error_string
        lua_pop(tlmodule->L, 1); //
        return -1;
    }
    else if(lua_isnil(tlmodule->L, -1))
    {
        *content_length = 0;
        lua_pop(tlmodule->L, 1); //
        return 0;
    }
    *content_length = lua_tointeger(tlmodule->L, -1); // content_length
    lua_pop(tlmodule->L, 1); //
    return 0;
}

int tucube_epoll_http_module_on_body_chunk(struct tucube_module* module, struct tucube_cldata* cldata, char* body_chunk, ssize_t body_chunk_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    lua_getglobal(tlmodule->L, "onBodyChunk"); // onBodyChunk
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1);
        return 0;
    }

    lua_getglobal(tlmodule->L, "requests"); // onBodyChunk requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // onBodyChunk requests client_socket
    lua_gettable(tlmodule->L, -2); // onBodyChunk requests request
    lua_remove(tlmodule->L, -2); // onBodyChunk request
    lua_pushlstring(tlmodule->L, body_chunk, body_chunk_size); // onBodyChunk request body_chunk
    if(lua_pcall(tlmodule->L, 2, 0, 0) != 0) //
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // error_string
        lua_pop(tlmodule->L, 1); //
        return -1;
    }

    return 0;
}

int tucube_epoll_http_module_on_body_finish(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    lua_getglobal(tlmodule->L, "onBodyFinish"); // onBodyFinish
    if(lua_isnil(tlmodule->L, -1))
    {
        lua_pop(tlmodule->L, 1);
        return 0;
    }

    lua_getglobal(tlmodule->L, "requests"); // onBodyFinish requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // onBodyFinish requests client_socket
    lua_gettable(tlmodule->L, -2); // onBodyFinish requests request
    lua_remove(tlmodule->L, -2); // onBodyFinish request
    if(lua_pcall(tlmodule->L, 1, 0, 0) != 0) //
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // error_string
        lua_pop(tlmodule->L, 1); //
        return -1;
    }

    return 0;
}

int tucube_epoll_http_module_on_request_finish(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "requests"); // requests
    lua_pushinteger(tlmodule->L, *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket); // requests client_socket
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

    if(lua_isnil(tlmodule->L, -1)) // requests request scriptPath 
    {
        lua_pop(tlmodule->L, 1); // requests request
	    lua_pushstring(tlmodule->L, "scriptPath"); // requests request "scriptPath"
        script_path = "";
        script_path_size = sizeof("") - 1;
        lua_pushstring(tlmodule->L, ""); // requests request "scriptPath" "" 
        lua_settable(tlmodule->L, -3); // requests request
    }
    else
    {
        script_path = lua_tolstring(tlmodule->L, -1, &script_path_size); // requests request scriptPath 
        if(strncmp(script_path + script_path_size - 1, "/", sizeof("/") - 1) == 0)
            return -1;
        lua_pop(tlmodule->L, 1); // requests request
    }

    if((path_info = strstr(request_uri, script_path)) != request_uri) // if request uri doesn't begin with script name
        return -1;

    path_info += script_path_size;

    if((query_string = strstr(path_info, "?")) != NULL) // check if there is query string
    {
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
    else
    {
        lua_pushstring(tlmodule->L, "pathInfo"); // requests request "pathInfo"
        if(strlen(path_info) != 0) // check if path info is not empty string
            lua_pushstring(tlmodule->L, path_info); // requests request "pathInfo" pathInfo 
        else
            lua_pushstring(tlmodule->L, "/"); // requests request "pathInfo" pathInfo 
        lua_settable(tlmodule->L, -3); // requests request
    }

    lua_getglobal(tlmodule->L, "onRequestFinish"); // requests request onRequestFinish
    if(lua_isnil(tlmodule->L, -1))
    {
        warnx("%s: %u: onRequestFinish() is not found in the script", __FILE__, __LINE__);
        return -1;
    }
    lua_pushvalue(tlmodule->L, -2); // requests request onRequestFinish request
    if(lua_pcall(tlmodule->L, 1, 3, 0) != 0) // requests request status_code headers body
    {
        warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); // requests request error_string
        lua_pop(tlmodule->L, 4); //
        return -1;
    }
    
    lua_remove(tlmodule->L, 1); // request status_code headers body
    lua_remove(tlmodule->L, 1); // status_code headers body
    if(!(lua_isnumber(tlmodule->L, -3) && lua_istable(tlmodule->L, -2) && (lua_isstring(tlmodule->L, -1) || lua_isfunction(tlmodule->L, -1) || lua_isnil(tlmodule->L, -1))))
    {
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

int tucube_epoll_http_module_get_status_code(struct tucube_module* module, struct tucube_cldata* cldata, int* status_code)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *status_code = lua_tointeger(tlmodule->L, -3); // status_code headers body

    return 0;
}

int tucube_epoll_http_module_prepare_get_header(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pushnil(tlmodule->L); // status_code headers body nil
    if(lua_next(tlmodule->L, -3) != 0) // status_code headers body new_header_field new_header_value
        return 1;
    return 0;
}

int tucube_epoll_http_module_get_header(struct tucube_module* module, struct tucube_cldata* cldata, const char** header_field, size_t* header_field_size, const char** header_value, size_t* header_value_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *header_field = lua_tolstring(tlmodule->L, -2, header_field_size); // status_code headers body header_field header_value
    *header_value = lua_tolstring(tlmodule->L, -1, header_value_size); // status_code headers body header_field header_value
    lua_pop(tlmodule->L, 1); // status_code headers body header_field
    if(lua_next(tlmodule->L, -3) != 0) // status_code headers body header_field ->  status_code headers body (new_header_field new_header_value || empty)
        return 1;
    return 0;
}

int tucube_epoll_http_module_prepare_get_body(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(lua_isfunction(tlmodule->L, -1))
    {
        GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L = lua_newthread(tlmodule->L); // status_code headers body thread
        return 1;
    }
    else if(lua_isstring(tlmodule->L, -1))
    {
        GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L = NULL;
        return 1;
    }
    else if(lua_isnil(tlmodule->L, -1))
        return 0;
    return -1;
}

int tucube_epoll_http_module_get_body(struct tucube_module* module, struct tucube_cldata* cldata, const char** body, size_t* body_size)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    if(lua_isfunction(tlmodule->L, -2)) // status_code headers body thread
    {
        if(lua_gettop(GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L) > 1)
            lua_pop(GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L, 1);
        int coroutine_result;
        lua_pushvalue(tlmodule->L, -2); // status_code headers body thread body
        lua_xmove(tlmodule->L, GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L, 1); // status_code headers body thread
        coroutine_result = lua_resume(GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L, tlmodule->L, 0); // status_code headers body thread
        *body = lua_tolstring(GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->L, -1, body_size); // status_code headers body thread
        if(coroutine_result == LUA_YIELD)
            return 1; // status_code headers body thread
        lua_pop(tlmodule->L, 1); // status_code headers body
        if(coroutine_result == 0)
            return 0;
        return -1;
    }
    else if(lua_isstring(tlmodule->L, -1)) // status_code headers body
    {
        *body = lua_tolstring(tlmodule->L, -1, body_size);
        return 0;
    }

    return -1;
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_cldata* cldata)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pop(tlmodule->L, lua_gettop(tlmodule->L)); //
    close(*GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket);
    *GONC_CAST(cldata->pointer, struct tucube_http_lua_cldata*)->client_socket = -1;
    free(cldata->pointer);
    free(cldata);

    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module)
{
    struct tucube_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);

    if(tlmodule != NULL)
    {
        lua_getglobal(tlmodule->L, "onDestroy"); // onDestroy
        if(lua_isnil(tlmodule->L, -1))
            lua_pop(tlmodule->L, 1); //
        else
        {
            if(lua_pcall(tlmodule->L, 0, 0, 0) != 0)
            {
                warnx("%s: %u: %s", __FILE__, __LINE__, lua_tostring(tlmodule->L, -1)); //error_string
                lua_pop(tlmodule->L, 1); //
            }
        }

        lua_close(tlmodule->L);
        free(tlmodule);
    }

    return 0;
}

int tucube_epoll_http_module_destroy(struct tucube_module* module)
{
    pthread_key_delete(*module->tlmodule_key);
    free(module->tlmodule_key);
    free(module);

    return 0;
}
