add_library(vinbero_mt_http_lua MODULE vinbero_mt_http_lua.c)
set_target_properties(vinbero_mt_http_lua PROPERTIES PREFIX "")
target_link_libraries(vinbero_mt_http_lua lua)
install(TARGETS vinbero_mt_http_lua
        LIBRARY DESTINATION lib/tucube)
