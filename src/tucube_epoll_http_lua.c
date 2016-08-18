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
    {
        warnx("%s: %u: Argument lua-script-path is required", __FILE__, __LINE__);
        pthread_exit(NULL);
    }

    struct tucube_epoll_http_lua_tlmodule* tlmodule = malloc(sizeof(struct tucube_epoll_http_lua_tlmodule));

    tlmodule->L = luaL_newstate();
    luaL_openlibs(tlmodule->L);

    lua_newtable(tlmodule->L);
    lua_setglobal(tlmodule->L, "clients");

    if(luaL_loadfile(tlmodule->L, lua_script_path) != LUA_OK)
    {
        warnx(EXIT_FAILURE, "%s: %u: luaL_loadfile() failed", __FILE__, __LINE__);
        pthread_exit(NULL);
    }
    lua_pcall(tlmodule->L, 0, 0, 0);
    lua_pop(tlmodule->L, 1);

    pthread_setspecific(*module->tlmodule_key, tlmodule);
    return 0;
}

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_tcp_epoll_cldata_list* cldata_list, int* client_socket)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    struct tucube_tcp_epoll_cldata* cldata = malloc(sizeof(struct tucube_tcp_epoll_cldata));
    GONC_LIST_ELEMENT_INIT(cldata);
    cldata->pointer = client_socket;
    GONC_LIST_APPEND(cldata_list, cldata);
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

    return 0;
}

int tucube_epoll_http_module_get_status_code(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, int* status_code)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *status_code = lua_tointeger(tlmodule->L, -3);
    lua_pushnil(tlmodule->L); // for lua_next() in get_header()
    lua_next(tlmodule->L, -3); // prepare for get_header()
    return 0;
}

int tucube_epoll_http_module_get_header(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, const char** header_field, size_t* header_field_size, const char** header_value, size_t* header_value_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    *header_field = lua_tolstring(tlmodule->L, -2, header_field_size);
    *header_value = lua_tolstring(tlmodule->L, -1, header_value_size);
    lua_pop(tlmodule->L, 1);
    if(lua_next(tlmodule->L, -3) != 0)
        return 1;
    return 0;
}

int tucube_epoll_http_module_get_body(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata, const char** body, size_t* body_size)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(lua_isfunction(tlmodule->L, -1))
    {
        lua_State* L = lua_newthread(tlmodule->L);
        lua_pop(tlmodule->L, 1);
        return -1; // currently not supported.
    }
    else
    {
        *body = lua_tolstring(tlmodule->L, -1, body_size);
        return 0;
    }
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    lua_pop(tlmodule->L, 3);
    close(*(int*)cldata->pointer);
    *(int*)cldata->pointer = -1;
    free(cldata);
    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module)
{
    struct tucube_epoll_http_lua_tlmodule* tlmodule = pthread_getspecific(*module->tlmodule_key);
    if(tlmodule != NULL)
    {
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
