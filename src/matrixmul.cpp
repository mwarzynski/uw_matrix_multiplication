#include "matrixmul.h"

namespace matrixmul {

void Algorithm::prepareMatrices(int seed) {
    matrixB = std::make_unique<matrix::Dense>(n, com->rank(), com->numProcesses(), seed);
    matrixC = std::make_unique<matrix::Dense>(n, com->rank(), com->numProcesses());
}

void Algorithm::initializeCoordinator(int matrix_n, std::vector<matrix::Sparse> matricesA) {
    n = matrix_n;
    com->BroadcastN(n);
    matrixA = std::make_unique<matrix::Sparse>(matricesA[0]);
    for (size_t i = 1; i < matricesA.size(); i++) {
        com->SendSparse(&matricesA[i], i, PHASE_INITIALIZATION);
    }
}

void Algorithm::initializeWorker() {
    n = com->ReceiveN();
    matrixA = com->ReceiveSparse(messaging::Communicator::rankCoordinator(), PHASE_INITIALIZATION);
}

AlgorithmCOLA::AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *communicator, int seed) {
    com = communicator;
    if (com->isCoordinator()) {
        initializeCoordinator(full_matrix->n, full_matrix->Split(com->numProcesses()));
    } else {
        initializeWorker();
    }
    prepareMatrices(seed);
}

void AlgorithmCOLA::phase_replication() {}

void AlgorithmCOLA::phase_computation() {}

void Algorithm::phase_final_ge() {}

void Algorithm::phase_final_matrix() {}

}
