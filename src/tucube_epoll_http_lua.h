#ifndef _TUCUBE_EPOLL_HTTP_LUA_H
#define _TUCUBE_EPOLL_HTTP_LUA_H

#include <lua.h>
#include <tucube/tucube_module.h>
#include <tucube/tucube_cldata.h>

struct tucube_epoll_http_lua_tlmodule
{
   lua_State* L;
};

struct tucube_epoll_http_lua_cldata
{
    int* client_socket;
    lua_State* L;
};

int tucube_epoll_http_lua_closef(lua_State* L);

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list);
int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args);
int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_cldata_list* cldata_list, int* cilent_socket);

int tucube_epoll_http_module_on_request_start(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_on_method(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size);
int tucube_epoll_http_module_on_uri(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size);
int tucube_epoll_http_module_on_version(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size);
int tucube_epoll_http_module_on_header_field(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size);
int tucube_epoll_http_module_on_header_value(struct tucube_module* module, struct tucube_cldata* cldata, char* token, ssize_t token_size);
int tucube_epoll_http_module_on_headers_finish(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_get_content_length(struct tucube_module* module, struct tucube_cldata* cldata, ssize_t* content_length);
int tucube_epoll_http_module_on_body_chunk(struct tucube_module* module, struct tucube_cldata* cldata, char* body_chunk, ssize_t body_chunk_size);
int tucube_epoll_http_module_on_body_finish(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_on_request_finish(struct tucube_module* module, struct tucube_cldata* cldata);

int tucube_epoll_http_module_get_status_code(struct tucube_module* module, struct tucube_cldata* cldata, int* status_code);

int tucube_epoll_http_module_prepare_get_header(struct tucube_module* module, struct tucube_cldata* cldata);

int tucube_epoll_http_module_get_header(struct tucube_module* module, struct tucube_cldata* cldata, const char** header_field, size_t* header_field_size, const char** header_value, size_t* header_value_size);

int tucube_epoll_http_module_prepare_get_body(struct tucube_module* module, struct tucube_cldata* cldata);

int tucube_epoll_http_module_get_body(struct tucube_module* module, struct tucube_cldata* cldata, const char** body, size_t* body_size);

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_cldata* cldata);
int tucube_epoll_http_module_tldestroy(struct tucube_module* module);
int tucube_epoll_http_module_destroy(struct tucube_module* module);

#endif
