cmake_minimum_required(VERSION 3.29)

# Project name
project(PER_FPGA)


set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS_DEBUG "-g -fopenmp")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fopenmp")


if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()



file(GLOB_RECURSE SRCS impl/*/)
include_directories(include)

add_executable(FPGAPeRMain FPGAPeRMain.cpp ${SRCS})

add_executable(QCAPeRMain QCAPeRMain.cpp ${SRCS})

add_executable(chDotStrToN chDotStrToNMain.cpp ${SRCS})

