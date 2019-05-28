#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include <memory>
#include "matrix.h"
#include "communicator.h"

// MKL - matrix multiplication of sparse and dense matrix.
// https://software.intel.com/en-us/mkl-developer-reference-fortran-mkl-sparse-mm

#define PHASE_INITIALIZATION 10
#define PHASE_COMPUTATION 10
#define PHASE_FINAL 10


namespace matrixmul {

enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class Algorithm {
public:
    int n;
    int c;
    messaging::Communicator *communicator;

    std::unique_ptr<matrix::Sparse> matrixA;
    std::unique_ptr<matrix::Dense> matrixB;
    std::unique_ptr<matrix::Dense> matrixC;

    void phase_replication();
    virtual void phase_computation(int power) = 0;

    void phase_final_matrix();
    void phase_final_ge(double g);

    void prepareMatrices(int seed);
    void initializeWorker();
    void initializeCoordinator(int n, std::vector<matrix::Sparse> matricesA);
};

class AlgorithmCOLA : public Algorithm {
public:
    AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com, int c, int seed);

    void phase_computation(int power) override;
private:
    void phase_computation_partial();
    void phase_computation_cycle_A(messaging::Communicator *comm);
};

class AlgorithmCOLABC : public Algorithm {
public:
    AlgorithmCOLABC(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *communicator, int seed) {};

    void phase_computation(int power) override {};
};

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
