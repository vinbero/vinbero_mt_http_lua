add_library(vinbero_mt_http_lua MODULE vinbero_mt_http_lua.c)
set_target_properties(vinbero_mt_http_lua PROPERTIES PREFIX "")
set_target_properties(vinbero_mt_http_lua PROPERTIES VERSION ${vinbero_mt_http_lua_version}
                                                     SOVERSION ${vinbero_mt_http_lua_soversion}
                                                     PUBLIC_HEADER "")
find_library(fastdl libfastdl.so REQUIRED)
find_library(gaio libgaio.so.0.3 REQUIRED)
find_library(vinbero_common libvinbero_common.so REQUIRED)
find_library(lua NAMES liblua-5.3.so liblua-5.3.so.0 REQUIRED)
target_link_libraries(vinbero_mt_http_lua ${vinbero_common} ${fastdl} ${gaio} ${lua})

install(TARGETS vinbero_mt_http_lua
        LIBRARY DESTINATION lib/vinbero
        PUBLIC_HEADER DESTINATION include/vinbero)
