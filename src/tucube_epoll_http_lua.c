#include <err.h>
#include <libgonc/gonc_list.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
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
    struct tucube_epoll_http_lua_tlmodule* tlmodule = malloc(sizeof(struct tucube_epoll_http_lua_tlmodule));
    tlmodule->L = luaL_newstate();
    luaL_openlibs(tlmodule->L);
    lua_newtable(tlmodule->L);
    lua_setglobal(tlmodule->L, "client_table");
/*
    if(luaL_loadfile(tlmodule->L, "test.lua") != LUA_OK)
        errx(EXIT_FAILURE, "%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
*/
    pthread_setspecific(*module->tlmodule_key, tlmodule);   
    return 0;
}

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_tcp_epoll_cldata_list* cldata_list, int client_socket)
{
    return 0;
}

int tucube_epoll_http_module_on_method(char* token, ssize_t token_size)
{
warnx("on_method()");
    char* method = malloc(token_size + 1);
    method[token_size] = '\0';
    memcpy(method, token, token_size);
    warnx("%s", method);
    free(method);
    return 0;
}

int tucube_epoll_http_module_on_uri(char* token, ssize_t token_size)
{
warnx("on_uri()");
    char* uri = malloc(token_size + 1);
    uri[token_size] = '\0';
    memcpy(uri, token, token_size);
    warnx("%s", uri);
    free(uri);
    return 0;
}

int tucube_epoll_http_module_on_version(char* token, ssize_t token_size)
{
warnx("on_version()");
    char* version = malloc(token_size + 1);
    version[token_size] = '\0';
    memcpy(version, token, token_size);
    warnx("%s", version);
    free(version);
    return 0;
}

int tucube_epoll_http_module_on_header_field(char* token, ssize_t token_size)
{
warnx("on_header_field()");
    char* header_field = malloc(token_size + 1);
    header_field[token_size] = '\0';
    memcpy(header_field, token, token_size);
    warnx("%s", header_field);
    free(header_field);
    return 0;
}

int tucube_epoll_http_module_on_header_value(char* token, ssize_t token_size)
{
warnx("on_header_value()");
    char* header_value = malloc(token_size + 1);
    header_value[token_size] = '\0';
    memcpy(header_value, token, token_size);
    warnx("%s", header_value);
    free(header_value);
    return 0;
}

int tucube_epoll_http_module_service(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
{
    return 0;
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
{
    warnx("tucube_epoll_http_module_cldestroy()");
    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_close(tlmodule->L);
    free(tlmodule);
    pthread_key_delete(*module->tlmodule_key);
    return 0;
}

int tucube_epoll_http_module_destroy(struct tucube_module* module)
{
    warnx("tucube_epoll_http_module_destroy()");
    free(module);
    return 0;
}
