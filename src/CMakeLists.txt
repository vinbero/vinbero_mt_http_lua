add_library(vinbero_mt_http_lua MODULE vinbero_mt_http_lua.c)
set_target_properties(
    vinbero_mt_http_lua
    PROPERTIES
    C_STANDARD 11
    PREFIX ""
    VERSION ${version}
    SOVERSION ${soversion}
    PUBLIC_HEADER ""
)
find_library(fastdl libfastdl.so REQUIRED)
find_library(gaio libgaio.so REQUIRED)
include(CMakeFindDependencyMacro)
find_dependency(vinbero_com 0.4 REQUIRED)

find_library(lua_LIBRARIES NAMES liblua5.3.so liblua-5.3.so liblua-5.3.so.0 liblua.a REQUIRED)
find_path(lua_INCLUDE_DIRS NAMES lua.h lualib.h lauxlib.h PATHS /usr/include/lua5.3 REQUIRED)

target_link_libraries(vinbero_mt_http_lua vinbero_com::vinbero_com ${fastdl} ${gaio} ${lua_LIBRARIES})
target_include_directories(vinbero_mt_http_lua PRIVATE ${lua_INCLUDE_DIRS})

install(TARGETS vinbero_mt_http_lua
        LIBRARY DESTINATION lib/vinbero
        PUBLIC_HEADER DESTINATION include/vinbero)
