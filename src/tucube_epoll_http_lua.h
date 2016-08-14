#ifndef _TUCUBE_EPOLL_HTTP_LUA_H
#define _TUCUBE_EPOLL_HTTP_LUA_H

#include <tucube/tucube_module.h>
#include "../../tucube_tcp_epoll/src/tucube_tcp_epoll_cldata.h"

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list);
int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args);
int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_tcp_epoll_cldata_list* cldata_list);

int tucube_epoll_http_module_on_method(char* token, ssize_t token_size);
int tucube_epoll_http_module_on_uri(char* token, ssize_t token_size);
int tucube_epoll_http_module_on_version(char* token, ssize_t token_size);
int tucube_epoll_http_module_on_header_field(char* token, ssize_t token_size);
int tucube_epoll_http_module_on_header_value(char* token, ssize_t token_size);

int tucube_epoll_http_module_service(struct tucube_module* module);

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata);
int tucube_epoll_http_module_tldestroy(struct tucube_module* module);
int tucube_epoll_http_module_destroy(struct tucube_module* module);

#endif
