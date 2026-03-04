cmake_minimum_required(VERSION 3.10)
project(Task1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(task2_1 var1.cpp)
add_executable(task2_2 var2.cpp)

target_link_libraries(task2_1 PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(task2_2 PRIVATE OpenMP::OpenMP_CXX)

# Включаем оптимизации
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(task2_1 PRIVATE -O3 -march=native -mtune=native)
    target_compile_options(task2_2 PRIVATE -O3 -march=native -mtune=native)
endif()