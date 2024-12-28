# Versão mínima do CMake
cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(PER_FPGA)

# Ativa os avisos para código moderno (opcional)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

file(GLOB_RECURSE SRCS impl/*.cpp)
include_directories(include)

# Adicionar o primeiro executável
add_executable(yoto_main yoto_main.cpp ${SRCS}
        include/util.h
        impl/util.cpp
        impl/graph.cpp
        include/graph.h)



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
