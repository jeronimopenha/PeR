cmake_minimum_required(VERSION 3.29)

# Project name
project(PER_FPGA)


set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

file(GLOB_RECURSE SRCS impl/*.cpp)
include_directories(include)

add_executable(PeRMain PeRMain.cpp ${SRCS})

#add_executable(yotoBaseDf yotoBaseDfMain.cpp ${SRCS})

#add_executable(yotoBaseDfP yotoBaseDfPMain.cpp ${SRCS})

#add_executable(yotoBaseZz yotoBaseZzMain.cpp ${SRCS})

#add_executable(yotoBaseZzCache yotoBaseZzCacheMain.cpp ${SRCS})

add_executable(chDotStrToN chDotStrToNMain.cpp ${SRCS})

