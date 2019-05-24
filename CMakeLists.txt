cmake_minimum_required(VERSION 3.5)

project(uw_matrix_multiplication)

set(CMAKE_CXX_STANDARD 11)
set(MATRIX_MUL_SRCS
        src/densematgen.cpp
        src/densematgen.h
        src/main.cpp)

add_executable(matrixmul ${MATRIX_MUL_SRCS})
