cmake_minimum_required(VERSION 3.17)
project(CCB_Assembler)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(lib)
include_directories(lib/FileWatcher)

add_executable(CCB_Assembler
        lib/FileWatcher/FileWatcher/FileWatcher.h
        lib/FileWatcher/FileWatcher/FileWatcherImpl.h
        lib/FileWatcher/FileWatcher/FileWatcherLinux.h
        lib/FileWatcher/FileWatcher/FileWatcherOSX.h
        lib/FileWatcher/FileWatcher/FileWatcherWin32.h
        lib/FileWatcher/FileWatcher.cpp
        lib/FileWatcher/FileWatcherLinux.cpp
        lib/FileWatcher/FileWatcherOSX.cpp
        lib/FileWatcher/FileWatcherWin32.cpp
        lib/cxxopt.hpp
        lib/termcolor.hpp
        assembler.h
        main.cpp)
