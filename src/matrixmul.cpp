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
    auto matrixA_copy = *matrixA;
    int divider = communicator->rank() / c;
    auto comm_replication = communicator->Split(divider);
    for (int i = 0; i < comm_replication.numProcesses(); i++) {
        if (i == comm_replication.rank()) {
            comm_replication.BroadcastSendSparse(&matrixA_copy);
        } else {
            auto b = comm_replication.BroadcastReceiveSparse(i);
            matrixA = std::make_unique<matrix::Sparse>(matrixA.get(), b.get());
        }
    }
}

void Algorithm::phase_final_ge() {}

void Algorithm::phase_final_matrix() {}

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

void AlgorithmCOLA::phase_computation_partial() {
    auto it = matrix::SparseIt(matrixA.get());
    auto b_range = matrixB->ColumnRange();
    while (it.Next()) {
        auto itv = it.Value();

        int ax = std::get<0>(itv);
        int ay = std::get<1>(itv);

        double av = std::get<2>(itv);
        for (int by = b_range.first; by <= b_range.second; by++) {
            double bv = matrixB->Get(ay, by);
            auto cv = av * bv;
            matrixC->ItemAdd(ax, by, cv);
        }
    }
}

void AlgorithmCOLA::phase_computation_cycle_A(messaging::Communicator *comm) {
    int sender = comm->rank() - 1;
    if (sender == -1) {
        sender = comm->numProcesses() - 1;
    }
    int receiver = (comm->rank() + 1) % (comm->numProcesses());
    if (comm->rank() % 2 == 0) {
        comm->SendSparse(matrixA.get(), receiver, PHASE_COMPUTATION);
        matrixA = comm->ReceiveSparse(sender, PHASE_COMPUTATION);
    } else {
        auto ma = comm->ReceiveSparse(sender, PHASE_COMPUTATION);
        comm->SendSparse(matrixA.get(), receiver, PHASE_COMPUTATION);
        matrixA = std::move(ma);
    }
}

void AlgorithmCOLA::phase_computation() {
    auto comm_computation = communicator->Split(communicator->rank() % c);
    for (int i = 0; i < comm_computation.numProcesses(); i++) {
        phase_computation_partial();
        phase_computation_cycle_A(&comm_computation);
    }
}

}
