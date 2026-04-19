cmake_minimum_required(VERSION 3.16)
project(ares_cdh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(ares_cdh STATIC
    src/crc.cpp
    src/parser.cpp
)

target_include_directories(ares_cdh
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_options(ares_cdh PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

include(CTest)

if(BUILD_TESTING AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_executable(ares_tests
        tests/test_crc.cpp
        tests/test_parser.cpp
        tests/catch_amalgamated.cpp
    )

    target_link_libraries(ares_tests PRIVATE ares_cdh)

    target_include_directories(ares_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/tests"
    )


    target_compile_options(ares_tests PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    )

    add_test(NAME ares_tests COMMAND ares_tests)
endif()