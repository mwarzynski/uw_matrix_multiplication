cmake_minimum_required(VERSION 2.8)

project(uw_matrix_multiplication)

SET(CMAKE_CXX_COMPILER mpic++)
SET(MATRIX_MUL_SRCS ./src/matrix_multiplication.cpp)

add_executable(matrixmul ${MATRIX_MUL_SRCS})
