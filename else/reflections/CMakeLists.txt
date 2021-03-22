cmake_minimum_required(VERSION 3.16)
project(reflections)
find_package(MPI)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
set(CMAKE_CXX_STANDARD 17)

add_executable(reflections main.cpp)
