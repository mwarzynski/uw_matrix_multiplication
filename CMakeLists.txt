cmake_minimum_required(VERSION 3.5)

project(uw_matrix_multiplication)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O3")

include_directories(include)

set(MATRIX_MUL_SRCS src/densematgen.cpp src/parser.cpp src/matrixmul.cpp src/communicator.cpp src/matrix.cpp src/main.cpp)

add_executable(matrixmul ${MATRIX_MUL_SRCS})
