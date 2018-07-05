add_library(vinbero_mt_http_lua MODULE vinbero_mt_http_lua.c)
set_target_properties(vinbero_mt_http_lua PROPERTIES PREFIX "")
set_target_properties(vinbero_mt_http_lua PROPERTIES VERSION ${vinbero_mt_http_lua_version}
                                                     SOVERSION ${vinbero_mt_http_lua_soversion}
                                                     PUBLIC_HEADER "")
install(TARGETS vinbero_mt_http_lua
        LIBRARY DESTINATION lib/vinbero
        PUBLIC_HEADER DESTINATION include/vinbero)
