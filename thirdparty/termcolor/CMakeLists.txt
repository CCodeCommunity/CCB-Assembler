cmake_minimum_required(VERSION 3.13)
project(termcolor)

add_library(termcolor INTERFACE)
target_include_directories(termcolor INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")