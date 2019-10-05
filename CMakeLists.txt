cmake_minimum_required(VERSION 3.5)
include(cmake/gonapps-cmake-helper.cmake)
project(vinbero_mt_http_lua C)

gonapps_setVersion("0.3.2" "0.3")
gonapps_setBuildType()

add_subdirectory(iface)
add_subdirectory(src)
enable_testing()
add_subdirectory(test)
