add_library(vinbero_mt_http_lua MODULE vinbero_mt_http_lua.c)
set_target_properties(vinbero_mt_http_lua PROPERTIES
                                          VERSION 0.1.0
                                          SOVERSION 0.1
                                          PREFIX "")
target_link_libraries(vinbero_mt_http_lua lua)
install(TARGETS vinbero_mt_http_lua
        LIBRARY DESTINATION lib/vinbero)
