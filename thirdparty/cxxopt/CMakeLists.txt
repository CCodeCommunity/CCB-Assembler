cmake_minimum_required(VERSION 3.13)
project(cxxopt)

add_library(cxxopt INTERFACE)
target_include_directories(cxxopt INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")