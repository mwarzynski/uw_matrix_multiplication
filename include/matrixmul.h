#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include <memory>
#include "matrix.h"
#include "communicator.h"

// MKL - matrix multiplication of sparse and dense matrix.
// https://software.intel.com/en-us/mkl-developer-reference-fortran-mkl-sparse-mm

namespace matrixmul {

enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class Algorithm {
public:

    virtual void replicate() = 0;
    virtual void compute() = 0;
};

class AlgorithmCOLA : public Algorithm {
public:
    AlgorithmCOLA(std::unique_ptr<matrix::Sparse> matrixA, messaging::Communicator *communicator, int seed);

    void replicate() override;
    void compute() override;

private:
    std::unique_ptr<matrix::Dense> matrixB;
    std::unique_ptr<matrix::Dense> matrixC;
    messaging::Communicator *com;
};

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
