#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <tucube/tucube_module.h>
#include <libgonc/gonc_list.h>
#include "../../tucube_tcp_epoll/src/tucube_tcp_epoll_cldata.h"

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

int tucube_epoll_http_module_clinit(struct tucube_module* module, struct tucube_tcp_epoll_cldata_list* cldata_list)
{
    return 0;
}


int tucube_epoll_http_module_on_method(char* token, ssize_t token_offset)
{
    char* method = malloc(sizeof(token_offset) + 1);
    method[token_offset] = '\0';
    memcpy(method, token, token_offset);
    warnx("%s", method);
    free(method);
    return 0;
}

int tucube_epoll_http_module_on_url(char* token, ssize_t token_offset)
{
    char* url = malloc(sizeof(token_offset) + 1);
    url[token_offset] = '\0';
    memcpy(url, token, token_offset);
    warnx("%s", url);
    free(url);
    return 0;
}

int tucube_epoll_http_module_on_protocol(char* token, ssize_t token_offset)
{
    char* protocol = malloc(sizeof(token_offset) + 1);
    protocol[token_offset] = '\0';
    memcpy(protocol, token, token_offset);
    warnx("%s", protocol);
    free(protocol);
    return 0;
}

int tucube_epoll_http_module_on_header_field(char* token, ssize_t token_offset)
{
    char* header_field = malloc(sizeof(token_offset) + 1);
    header_field[token_offset] = '\0';
    memcpy(header_field, token, token_offset);
    warnx("%s", header_field);
    free(header_field);
    return 0;
}

int tucube_epoll_http_module_on_header_value(char* token, ssize_t token_offset)
{
    char* header_value = malloc(sizeof(token_offset) + 1);
    header_value[token_offset] = '\0';
    memcpy(header_value, token, token_offset);
    warnx("%s", header_value);
    free(header_value);
    return 0;
}

int tucube_epoll_http_module_service(struct tucube_module* module)
{
    return 0;
}

int tucube_epoll_http_module_cldestroy(struct tucube_module* module, struct tucube_tcp_epoll_cldata* cldata)
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
