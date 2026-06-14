cmake_minimum_required(VERSION 3.10)
project(Officials)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0601)
endif()

set(SOURCES
    main.cpp
    ministry.cpp
)

set(HEADERS
    ministry.h
)

add_executable(officials ${SOURCES} ${HEADERS})