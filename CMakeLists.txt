cmake_minimum_required(VERSION 3.14)
project(tape_sort CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(tape_sort_lib
        TapeConfig.cpp
        FileTape.cpp
        TapeSorter.cpp
)
target_include_directories(tape_sort_lib PUBLIC include)

find_package(Threads REQUIRED)
target_link_libraries(tape_sort_lib PUBLIC Threads::Threads)

add_executable(tape_sort main.cpp)
target_link_libraries(tape_sort PRIVATE tape_sort_lib)