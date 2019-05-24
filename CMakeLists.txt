cmake_minimum_required(VERSION 2.8)

project(uw_matrix_multiplication)

SET(CMAKE_CXX_STANDARD 11)
SET(MATRIX_MUL_SRCS src/main.cpp)

add_executable(matrixmul ${MATRIX_MUL_SRCS})
