#include "matrixmul.h"

namespace matrixmul {

void Algorithm::prepareMatrices(int seed) {
    matrixB = std::make_unique<matrix::Dense>(n, com->rank(), com->numProcesses(), seed);
    matrixC = std::make_unique<matrix::Dense>(n, com->rank(), com->numProcesses());
}

void Algorithm::initializeCoordinator(int matrix_n, std::vector<matrix::Sparse> matricesA) {
    n = matrix_n;
    com->BroadcastN(n);
    for (size_t i = 0; i < matricesA.size(); i++) {
        // Send matricesA[i] to corresponding worker.
    }
}

void Algorithm::initializeWorker() {
    n = com->ReceiveN();
}

AlgorithmCOLA::AlgorithmCOLA(std::unique_ptr<matrix::Sparse> matrixA, messaging::Communicator *communicator, int seed) {
    com = communicator;
    if (com->isCoordinator()) {
        initializeCoordinator(matrixA->n, matrixA->Split(com->numProcesses()));
    } else {
        initializeWorker();
    }
    prepareMatrices(seed);
}

void AlgorithmCOLA::phase_replication() {}

void AlgorithmCOLA::phase_computation() {}

}