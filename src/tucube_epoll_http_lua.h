#ifndef _TUCUBE_EPOLL_HTTP_LUA_H
#define _TUCUBE_EPOLL_HTTP_LUA_H

#include <tucube/tucube_module.h>
#include "../../tucube_epoll/src/tucube_epoll_cldata.h"

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list);
int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args);
int tucube_epoll_http_module_clinit();

int tucube_epoll_http_module_on_method_end();
int tucube_epoll_http_module_on_url_end();
int tucube_epoll_http_modue_on_version_end();
int tucube_epoll_http_module_on_header_field_end();
int tucube_epoll_http_module_on_header_value_end();

int tucube_epoll_http_module_service(struct tucube_module* module, int* client_socket);

int tucube_epoll_http_module_cldestroy();
int tucube_epoll_http_module_tldestroy(struct tucube_module* module);
int tucube_epoll_http_module_destroy(struct tucube_module* module);

#endif
