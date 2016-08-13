#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <tucube/tucube_module.h>
#include <libgonc/gonc_list.h>
#include "../../tucube_epoll/src/tucube_epoll_cldata.h"

int tucube_epoll_http_module_init(struct tucube_module_args* module_args, struct tucube_module_list* module_list)
{
    struct tucube_module* module = malloc(sizeof(struct tucube_module));
    GONC_LIST_ELEMENT_INIT(module);
    GONC_LIST_APPEND(module_list, module);
    return 0;
}

int tucube_epoll_http_module_tlinit(struct tucube_module* module, struct tucube_module_args* module_args)
{
    return 0;
}

int tucube_epoll_http_module_on_method()
{
}

int tucube_epoll_http_module_on_url()
{
}

int tucube_epoll_http_modue_on_protocol()
{
}

int tucube_epoll_http_module_on_header_field()
{
}

int tucube_epoll_http_module_on_header_value()
{
}

int tucube_epoll_http_module_service(struct tucube_module* module, int* client_socket)
{
    return 0;
}

int tucube_epoll_http_module_tldestroy(struct tucube_module* module)
{
    warnx("tucube_epoll_http_module_tldestroy()");
    return 0;
}

int tucube_epoll_http_module_destroy(struct tucube_module* module)
{
    free(module);
    return 0;
}
