cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

project(
    base57
    LANGUAGES C
)

add_library(
    base57
    "base57.c"
)

add_executable(
    base57test
    "base57test.c"
)

target_link_libraries(
    base57test
    base57
)

add_executable(
    base57encode
    "base57encode.c"
)

target_link_libraries(
    base57encode
    base57
)

add_executable(
    base57decode
    "base57decode.c"
)

target_link_libraries(
    base57decode
    base57
)
