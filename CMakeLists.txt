cmake_minimum_required(VERSION 3.10)
project(cutils VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_library(cutils STATIC)
target_sources(cutils
    PRIVATE
        src/cutils/array_list.c
        src/cutils/errors.c
        src/cutils/hashmap.c
        src/cutils/json.c
        src/cutils/linked_list.c
        src/cutils/md5.c
)
target_include_directories(cutils
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

enable_testing()
add_subdirectory(tests)
