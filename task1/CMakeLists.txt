cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(book_tests VERSION 0.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(myproject
    task1.cpp
)

target_include_directories(myproject PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})