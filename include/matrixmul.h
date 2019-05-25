#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include "matrix.h"
#include "communicator.h"

// MKL - matrix multiplication of sparse and dense matrix.
// https://software.intel.com/en-us/mkl-developer-reference-fortran-mkl-sparse-mm


enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class AlgorithmCOLA {
public:
    AlgorithmCOLA(matrix::Sparse *matrixA, messaging::Communicator *communicator, int seed);

private:
    messaging::Communicator *com;
};

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
