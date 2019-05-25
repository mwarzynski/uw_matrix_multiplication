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
    int n;
    messaging::Communicator *com;

    std::unique_ptr<matrix::Sparse> matrixA;
    std::unique_ptr<matrix::Dense> matrixB;
    std::unique_ptr<matrix::Dense> matrixC;

    virtual void phase_replication() = 0;
    virtual void phase_computation() = 0;

    void prepareMatrices(int seed);
    void initializeWorker();
    void initializeCoordinator(int n, std::vector<matrix::Sparse> matricesA);
};

class AlgorithmCOLA : public Algorithm {
public:
    AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *communicator, int seed);

    void phase_replication() override;
    void phase_computation() override;

private:

};

class AlgorithmCOLABC : public Algorithm {
public:
    AlgorithmCOLABC(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *communicator, int seed) {};

    void phase_replication() override {};
    void phase_computation() override {};
};

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
