cmake_minimum_required(VERSION 2.8)
project(vinbero_mt_http_lua C)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(version "0.3.0")
set(soversion "0.3")
string(REPLACE "." ";" version_list ${version})
list(GET version_list 0 version_major)
list(GET version_list 1 version_minor)
list(GET version_list 2 version_patch)

if(CMAKE_VERSION VERSION_LESS "3.1")
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_C_FLAGS "-std=gnu11 ${CMAKE_C_FLAGS}")
  endif()
else()
    set(CMAKE_C_STANDARD 11)
endif()

configure_file("${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME}_Version.h.in"
               "${PROJECT_BINARY_DIR}/${PROJECT_NAME}_Version.h" @ONLY)
include_directories(${PROJECT_BINARY_DIR})

execute_process(COMMAND git remote set-branches origin "*")
execute_process(COMMAND git fetch)
execute_process(COMMAND git shortlog -nse --no-merges origin/master OUTPUT_FILE "${CMAKE_SOURCE_DIR}/AUTHORS")
add_custom_target(authors ALL git shortlog -nse --no-merges origin/master > "${CMAKE_SOURCE_DIR}/AUTHORS")

add_subdirectory(src)
enable_testing()
add_subdirectory(test)
