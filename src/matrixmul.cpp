#include "matrixmul.h"

namespace matrixmul {

void Algorithm::prepareMatrices(int seed) {
    matrixB = std::make_unique<matrix::Dense>(n, communicator->rank(), communicator->numProcesses(), seed);
    matrixC = std::make_unique<matrix::Dense>(n, communicator->rank(), communicator->numProcesses());
}

void Algorithm::initializeCoordinator(int matrix_n, std::vector<matrix::Sparse> matricesA) {
    n = matrix_n;
    communicator->BroadcastN(n);
    matrixA = std::make_unique<matrix::Sparse>(matricesA[0]);
    for (size_t i = 1; i < matricesA.size(); i++) {
        communicator->SendSparse(&matricesA[i], i, PHASE_INITIALIZATION);
    }
}

void Algorithm::initializeWorker() {
    n = communicator->ReceiveN();
    matrixA = communicator->ReceiveSparse(messaging::Communicator::rankCoordinator(), PHASE_INITIALIZATION);
}

void Algorithm::phase_replication() {
    auto matrixA_base = *matrixA;
    int divider = communicator->rank() / c;
    auto comm_replication = communicator->Split(divider);
    for (int i = 0; i < comm_replication.numProcesses(); i++) {
        if (i == comm_replication.rank()) {
            comm_replication.BroadcastSendSparse(&matrixA_base);
        } else {
            auto b = comm_replication.BroadcastReceiveSparse(i);
            matrixA = std::make_unique<matrix::Sparse>(matrixA.get(), b.get());
        }
    }
}

AlgorithmCOLA::AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com,
    int rf, int seed) {
    communicator = com;
    c = rf;
    if (communicator->isCoordinator()) {
        initializeCoordinator(full_matrix->n, full_matrix->SplitColumns(communicator->numProcesses()));
    } else {
        initializeWorker();
    }
    prepareMatrices(seed);
}

void AlgorithmCOLA::phase_computation() {}

void Algorithm::phase_final_ge() {}

void Algorithm::phase_final_matrix() {}

}
