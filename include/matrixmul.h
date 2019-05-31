#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include <memory>
#include <cassert>
#include "matrix.h"
#include "communicator.h"

// MKL - matrix multiplication of sparse and dense matrix.
// https://software.intel.com/en-us/mkl-developer-reference-fortran-mkl-sparse-mm

#define PHASE_INITIALIZATION 1
#define PHASE_COMPUTATION 3
#define PHASE_FINAL 4


namespace matrixmul {

enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class Algorithm {
public:
    int n_original;
    int n;
    int c;

    messaging::Communicator *communicator;

    std::unique_ptr<matrix::Sparse> matrixA;
    std::unique_ptr<matrix::Dense> matrixB;
    std::unique_ptr<matrix::Dense> matrixC;

    Algorithm(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com, int replication_factor,
              int seed, bool split_by_columns);

    virtual void phaseReplication() = 0;
    virtual void phaseComputation(int power) = 0;
    virtual void phaseFinalMatrix() = 0;

    void phaseComputationPartial();
    void phaseComputationCycleA(messaging::Communicator *comm);
    void phaseFinalGE(double g);
};

class AlgorithmCOLA : public Algorithm {
public:
    AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com, int replication_factor,
        int seed);

    void phaseReplication() override;
    void phaseComputation(int power) override;
    void phaseFinalMatrix() override;
};

class AlgorithmCOLABC : public Algorithm {
public:
    AlgorithmCOLABC(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com, int replication_factor,
        int seed);

    void phaseReplication() override;
    void phaseComputation(int power) override;
    void phaseFinalMatrix() override;
};

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
