add_library(vinbero_mt_http_lua MODULE)

target_sources(
    vinbero_mt_http_lua
    PRIVATE
    vinbero_mt_http_lua.c
)

target_compile_options(vinbero_mt_http_lua PRIVATE -Wall)

set_target_properties(
    vinbero_mt_http_lua
    PROPERTIES
    C_STANDARD 11
    PREFIX ""
    VERSION ${version}
    SOVERSION ${soversion}
    PUBLIC_HEADER ""
)

find_package(fastdl REQUIRED)
find_package(gaio REQUIRED)
find_package(vinbero_com REQUIRED)

find_library(lua_LIBRARIES NAMES liblua5.3.so liblua-5.3.so liblua-5.3.so.0 liblua.a REQUIRED)
find_path(lua_INCLUDE_DIRS NAMES lua.h lualib.h lauxlib.h PATHS /usr/include/lua5.3 REQUIRED)

target_link_libraries(
    vinbero_mt_http_lua
    vinbero_com::vinbero_com
    fastdl::fastdl
    gaio::gaio
    ${lua_LIBRARIES}
)

target_include_directories(
    vinbero_mt_http_lua
    PRIVATE
    ${lua_INCLUDE_DIRS}
)

install(
    TARGETS
    vinbero_mt_http_lua
    LIBRARY DESTINATION lib/vinbero
    PUBLIC_HEADER DESTINATION include/vinbero
)
