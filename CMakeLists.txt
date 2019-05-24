cmake_minimum_required(VERSION 3.5)

project(uw_matrix_multiplication)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

set(MATRIX_MUL_SRCS src/densematgen.cpp src/parser.cpp src/matrixmul.cpp src/main.cpp)

add_executable(matrixmul ${MATRIX_MUL_SRCS})
