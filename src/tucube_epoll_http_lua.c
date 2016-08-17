#include <err.h>
#include <libgonc/gonc_list.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <tucube/tucube_module.h>
#include "../../tucube_tcp_epoll/src/tucube_tcp_epoll_cldata.h"
#include "tucube_epoll_http_lua.h"

const char* status_codes[600];

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list)
{
    struct tucube_module* module = malloc(sizeof(struct tucube_module));
    GONC_LIST_ELEMENT_INIT(module);
    module->tlmodule_key = malloc(sizeof(pthread_key_t));
    pthread_key_create(module->tlmodule_key, NULL);
    GONC_LIST_APPEND(module_list, module);

status_codes[100] = "Continue";
status_codes[101] = "Switching Protocols";
status_codes[102] = "Processing";

status_codes[200] = "OK";
status_codes[201] = "Created";
status_codes[202] = "Accepted";
status_codes[203] = "Non-authoritative Information";
status_codes[204] = "No Content";
status_codes[205] = "Reset Content";
status_codes[206] = "Partial Content";
status_codes[207] = "Multi-Status";
status_codes[208] = "Already Reported";
status_codes[226] = "IM Used";

status_codes[300] = "Multiple Choices";
status_codes[301] = "Moved Permanently";
status_codes[302] = "Found";
status_codes[303] = "See Other";
status_codes[304] = "Not Modified";
status_codes[305] = "Use Proxy";
status_codes[307] = "Temporary Redirect";
status_codes[308] = "Permanent Redirect";

status_codes[400] = "Bad Request";
status_codes[401] = "Unauthorized";
status_codes[402] = "Payment Required";
status_codes[403] = "Forbidden";
status_codes[404] = "Not Found";
status_codes[405] = "Method Not Allowed";
status_codes[406] = "Not Acceptable";
status_codes[407] = "Proxy Authentication Required";
status_codes[408] = "Request Timeout";
status_codes[409] = "Conflict";
status_codes[410] = "Gone";
status_codes[411] = "Length Required";
status_codes[412] = "Precondition Failed";
status_codes[413] = "Payload Too Large";
status_codes[414] = "Request-URI Too Long";
status_codes[415] = "Unsupported Media Type";
status_codes[416] = "Requested Range Not Satisfiable";
status_codes[417] = "Expectation Failed";
status_codes[418] = "I'm a teapot";
status_codes[421] = "Misdirected Request";
status_codes[422] = "Unprocessable Entity";
status_codes[423] = "Locked";
status_codes[424] = "Failed Dependency";
status_codes[426] = "Upgrade Required";
status_codes[428] = "Precondition Required";
status_codes[429] = "Too Many Requests";
status_codes[431] = "Request Header Fields Too Large";
status_codes[444] = "Connection Closed Without Response";
status_codes[451] = "Unavailable For Legal Reasons";
status_codes[499] = "Client Closed Request";

status_codes[500] = "Internal Server Error";
status_codes[501] = "Not Implemented";
status_codes[502] = "Bad Gateway";
status_codes[503] = "Service Unavailable";
status_codes[504] = "Gateway Timeout";
status_codes[505] = "HTTP Version Not Supported";
status_codes[506] = "Variant Also Negotiates";
status_codes[507] = "Insufficient Storage";
status_codes[508] = "Loop Detected";
status_codes[510] = "Not Extended";
status_codes[511] = "Network Authentication Required";
status_codes[599] = "Network Connect Timeout Error";

    return 0;
}

int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args)
{
    char* lua_script_path = NULL;
    GONC_LIST_FOR_EACH(module_args, struct tucube_module_arg, module_arg)
    {
        if(strncmp("lua-script-path", module_arg->name, sizeof("lua-script-path") - 1) == 0)
	     lua_script_path = module_arg->value;
    }

    if(lua_script_path == NULL)    
        errx(EXIT_FAILURE, "%s: %u: Argument lua-script-path is required", __FILE__, __LINE__);

    struct tucube_epoll_http_lua_tlmodule* tlmodule = malloc(sizeof(struct tucube_epoll_http_lua_tlmodule));

    tlmodule->L = luaL_newstate();
    luaL_openlibs(tlmodule->L);

    lua_newtable(tlmodule->L);
    lua_setglobal(tlmodule->L, "clients");

    if(luaL_loadfile(tlmodule->L, lua_script_path) != LUA_OK)
        errx(EXIT_FAILURE, "%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
    lua_pcall(tlmodule->L, 0, 0, 0);
    lua_pop(tlmodule->L, 1);

    pthread_setspecific(*module->tlmodule_key, tlmodule);   
    return 0;
}

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_tcp_epoll_cldata_list* cldata_list, int* client_socket)
{
    struct tucube_tcp_epoll_cldata* cldata = malloc(sizeof(struct tucube_tcp_epoll_cldata));
    GONC_LIST_ELEMENT_INIT(cldata);
    cldata->pointer = client_socket;
    GONC_LIST_APPEND(cldata_list, cldata);
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *client_socket);
    lua_newtable(tlmodule->L);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_method(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "REQUEST_METHOD");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_uri(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "REQUEST_URI");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_version(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "SERVER_PROTOCOL");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_header_field(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "recent_header_field");

    for(ssize_t index = 0; index < token_size; ++index)
    {
        if('a' <= token[index] && token[index] <= 'z')
            token[index] -= ('a' - 'A');
        else if(token[index] == '-')
            token[index] = '_';
    }

    if(strncmp("CONTENT_TYPE", token, sizeof("CONTENT_TYPE") - 1) == 0 ||
         strncmp("CONTENT_LENGTH", token, sizeof("CONTENT_LENGTH") - 1) == 0)
    {
        lua_pushlstring(tlmodule->L, token, token_size);
    }
    else
    {
        char* header_field;
        header_field = malloc(sizeof("HTTP_") - 1 + token_size);
        memcpy(header_field, "HTTP_", sizeof("HTTP_") - 1);
        memcpy(header_field + sizeof("HTTP_") - 1, token, token_size);
        lua_pushlstring(tlmodule->L, header_field, sizeof("HTTP_") - 1 + token_size);
        free(header_field);
    }
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_header_value(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "recent_header_field");
    lua_gettable(tlmodule->L, -2);
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pushstring(tlmodule->L, "recent_header_field");
    lua_pushnil(tlmodule->L);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}


int tucube_epoll_http_module_service(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "clients");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_getglobal(tlmodule->L, "service");
    lua_pushvalue(tlmodule->L, -2);
    lua_pcall(tlmodule->L, 1, 3, 0);
    lua_remove(tlmodule->L, -4);

//status line
    size_t status_code_size;
    const char* status_code = lua_tolstring(tlmodule->L, -3, &status_code_size);
    int status_code_integer = lua_tointeger(tlmodule->L, -3);
    write(*(int*)cldata->pointer, "HTTP/1.1", sizeof("HTTP/1.1") - 1);
    write(*(int*)cldata->pointer, " ", sizeof(" ") - 1);
    write(*(int*)cldata->pointer, status_code, status_code_size);
    write(*(int*)cldata->pointer, " ", sizeof(" ") - 1);
    write(*(int*)cldata->pointer, status_codes[status_code_integer], strlen(status_codes[status_code_integer]));
    write(*(int*)cldata->pointer, "\r\n", sizeof("\r\n") - 1);

//response headers
    lua_pushnil(tlmodule->L);
    while(lua_next(tlmodule->L, -3) != 0)
    {
        size_t response_header_field_size;
        const char* response_header_field = lua_tolstring(tlmodule->L, -2, &response_header_field_size);
        size_t response_header_value_size;
        const char* response_header_value = lua_tolstring(tlmodule->L, -1, &response_header_value_size);
        write(*(int*)cldata->pointer, response_header_field, response_header_field_size);
        write(*(int*)cldata->pointer, ": ", sizeof(": ") - 1);
        write(*(int*)cldata->pointer, response_header_value, response_header_value_size);
        write(*(int*)cldata->pointer, "\r\n", sizeof("\r\n") - 1);
        lua_pop(tlmodule->L, 1);
    }
    write(*(int*)cldata->pointer, "\r\n", sizeof("\r\n") - 1);

//response body
    if(lua_isfunction(tlmodule->L, -1))
    {
        lua_State* L = lua_newthread(tlmodule->L);
        lua_pop(tlmodule->L, 1);
    }
    else
    {
        size_t response_body_size;
        const char* response_body = lua_tolstring(tlmodule->L, -1, &response_body_size);
        write(*(int*)cldata->pointer, response_body, response_body_size);
    }

    lua_pop(tlmodule->L, 3);

    return 0;
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
{
    close(*(int*)cldata->pointer);
    *(int*)cldata->pointer = -1;
    free(cldata);
    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_close(tlmodule->L);
    free(tlmodule);
    return 0;
}

int tucube_epoll_http_module_destroy(struct tucube_module* module)
{
    pthread_key_delete(*module->tlmodule_key);
    free(module->tlmodule_key);
    free(module);
    return 0;
}
