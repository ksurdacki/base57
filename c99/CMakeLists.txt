cmake_minimum_required(VERSION 3.1)

set_property(GLOBAL PROPERTY C_STANDARD 99)
set_property(GLOBAL PROPERTY C_STANDARD_REQUIRED ON)

project(
    base57
    LANGUAGES C
)

add_library(
    base57
    base57.c
)

add_executable(
    test_base57
    test_base57.c
)

target_link_libraries(
    test_base57
    base57
)
