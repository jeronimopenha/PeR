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

add_executable(yotoBase yotoBaseMain.cpp ${SRCS})

add_executable(yotoCache yotoCacheMain.cpp ${SRCS})

add_executable(chDotStrToN chDotStrToNMain.cpp ${SRCS})


#cmake_minimum_required(VERSION 3.29)
#project(PeR)

#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/boost.cmake)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#if (NOT CMAKE_BUILD_TYPE)
#    set(CMAKE_BUILD_TYPE Debug)
#endif ()

#file(GLOB_RECURSE SRCS impl/*.cpp)

#include_directories(include)

#add_executable(PeR main.cpp ${SRCS}
#        impl/fpgaper.cpp
#        include/fpgaper.h)

#target_link_libraries(
#        PeR
#        PUBLIC Boost::pool
#)

#add_boost()
