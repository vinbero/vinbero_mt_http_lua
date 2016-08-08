#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <tcpcube/tcpcube_module.h>
#include <libgonc/gonc_list.h>

int tcpcube_epoll_http_module_init(struct tcpcube_module_args* module_args, struct tcpcube_module_list* module_list)
{
    struct tcpcube_module* module = malloc(sizeof(struct tcpcube_module));
    GONC_LIST_ELEMENT_INIT(module);
    GONC_LIST_APPEND(module_list, module);
    return 0;
}

int tcpcube_epoll_http_module_tlinit(struct tcpcube_module* module)
{
    return 0;
}

int tcpcube_epoll_http_module_on_method_end()
{
}

int tcpcube_epoll_http_module_on_url_end()
{
}

int tcpcube_epoll_http_modue_on_version_end()
{
}

int tcpcube_epoll_http_module_on_header_field_end()
{
}

int tcpcube_epoll_http_module_on_header_value_end()
{
}

int tcpcube_epoll_http_module_service(struct tcpcube_module* module, int* client_socket)
{
    return 0;
}

int tcpcube_epoll_http_module_tldestroy(struct tcpcube_module* module)
{
    warnx("tcpcube_epoll_http_module_tldestroy()");
    return 0;
}

int tcpcube_epoll_http_module_destroy(struct tcpcube_module* module)
{
    free(module);
    return 0;
}
