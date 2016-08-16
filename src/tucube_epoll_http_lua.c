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

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list)
{
    struct tucube_module* module = malloc(sizeof(struct tucube_module));
    GONC_LIST_ELEMENT_INIT(module);
    module->tlmodule_key = malloc(sizeof(pthread_key_t));
    pthread_key_create(module->tlmodule_key, NULL);
    GONC_LIST_APPEND(module_list, module);
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
    lua_setglobal(tlmodule->L, "client_table");

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
    lua_getglobal(tlmodule->L, "client_table");
    lua_pushinteger(tlmodule->L, *client_socket);
    lua_newtable(tlmodule->L);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_method(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "client_table");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "METHOD");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_uri(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "client_table");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "URI");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_version(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "client_table");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "VERSION");
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_header_field(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "client_table");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "recent_header_field");

    char* header_field = malloc(sizeof("HEADER_") - 1 + token_size);
    for(ssize_t index = 0; index < token_size; ++index)
    {
        if('a' <= token[index] && token[index] <= 'z')
            token[index] -= ('a' - 'A');
        else if(token[index] == '-')
            token[index] = '_';
    }
    memcpy(header_field, "HEADER_", sizeof("HEADER_") - 1);
    memcpy(header_field + sizeof("HEADER_") - 1, token, token_size);
    lua_pushlstring(tlmodule->L, header_field, sizeof("HEADER_") - 1 + token_size);
    free(header_field);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_on_header_value(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, char* token, ssize_t token_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "client_table");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_gettable(tlmodule->L, -2);
    lua_remove(tlmodule->L, -2);
    lua_pushstring(tlmodule->L, "recent_header_field");
    lua_gettable(tlmodule->L, -2);
    lua_pushlstring(tlmodule->L, token, token_size);
    lua_settable(tlmodule->L, -3);
    lua_pop(tlmodule->L, 1);
    return 0;
}

int tucube_epoll_http_module_service(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_getglobal(tlmodule->L, "service");
    lua_pushinteger(tlmodule->L, *(int*)cldata->pointer);
    lua_pcall(tlmodule->L, 1, 1, 0);
    const char* result;
    size_t result_size;
    result = lua_tolstring(tlmodule->L, -1, &result_size);
    write(*(int*)cldata->pointer, result, result_size);    
    lua_pop(tlmodule->L, 1);
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
