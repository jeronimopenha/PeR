cmake_minimum_required(VERSION 3.29)

project(PER_FPGA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_DEBUG "-g -fopenmp")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fopenmp")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif ()

include_directories(include)

# Fontes comuns
file(GLOB_RECURSE COMMON_SRCS impl/common/*.cpp)

# FPGA Executável
file(GLOB_RECURSE FPGA_SRCS impl/fpga/*.cpp)
add_executable(FPGAPeRMain FPGAPeRMain.cpp ${COMMON_SRCS} ${FPGA_SRCS})

# QCA Executável
file(GLOB_RECURSE QCA_SRCS impl/qca/*.cpp)
add_executable(QCAPeRMain QCAPeRMain.cpp ${COMMON_SRCS} ${QCA_SRCS})

# Ferramentas / Outros
#file(GLOB_RECURSE TOOL_SRCS impl/tools/*.cpp)
add_executable(chDotStrToN chDotStrToNMain.cpp ${COMMON_SRCS})


add_executable(rearrange rearrangeInOut.cpp ${COMMON_SRCS})