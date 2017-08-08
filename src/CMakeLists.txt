add_library(tucube_http_lua SHARED tucube_http_lua.c)
set_target_properties(tucube_http_lua PROPERTIES PREFIX "")
target_link_libraries(tucube_http_lua lua)
install(TARGETS tucube_http_lua DESTINATION /usr/local/lib/tucube/)
