#ifndef UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
#define UW_MATRIX_MULTIPLICATION_MATRIXMUL_H

#include <memory>
#include <cassert>
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

    void phaseReplication();
    virtual void phaseComputation(int power) = 0;

    void phaseFinalMatrix();
    void phaseFinalGE(double g);

    void prepareMatrices(int seed);
    void initializeWorker();
    void initializeCoordinator(int n, std::vector<matrix::Sparse> matricesA);
};

class AlgorithmCOLA : public Algorithm {
public:
    AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com, int c, int seed);

    void phaseComputation(int power) override;
private:
    void phaseComputationPartial();
    void phaseComputationCycleA(messaging::Communicator *comm);
};

class AlgorithmCOLABC : public Algorithm {
public:
    AlgorithmCOLABC(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *communicator, int seed) {};

    void phaseComputation(int power) override {};
};

}

#endif //UW_MATRIX_MULTIPLICATION_MATRIXMUL_H
