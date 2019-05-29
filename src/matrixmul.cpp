#include "matrixmul.h"

namespace matrixmul {

void Algorithm::prepareMatrices(int seed) {
    matrixB = std::make_unique<matrix::Dense>(n, communicator->rank(), communicator->numProcesses(), seed);
    matrixC = std::make_unique<matrix::Dense>(n, communicator->rank(), communicator->numProcesses());
}

void Algorithm::initializeCoordinator(int matrix_n, std::vector<matrix::Sparse> matricesA) {
    n = matrix_n;
    communicator->BroadcastSendN(n);
    matrixA = std::make_unique<matrix::Sparse>(matricesA[0]);
    for (size_t i = 1; i < matricesA.size(); i++) {
        communicator->SendSparse(&matricesA[i], i, PHASE_INITIALIZATION);
    }
}

void Algorithm::initializeWorker() {
    n = communicator->BroadcastReceiveN();
    matrixA = communicator->ReceiveSparse(communicator->rankCoordinator(), PHASE_INITIALIZATION);
}

void Algorithm::phaseReplication() {
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

void Algorithm::phaseFinalGE(double g) {
    long counter = 0;
    for (const auto &v : matrixC->values) {
        if (v >= g)
            counter++;
    }
    if (communicator->isCoordinator()) {
        // TODO: use MPI Reduce
        // https://mpitutorial.com/tutorials/mpi-reduce-and-allreduce/
        for (int p = 1; p < communicator->numProcesses(); p++)
            counter += communicator->ReceiveN(p, PHASE_FINAL);
        // Print out the result to stdout.
        std::cout << counter << std::endl;
    } else {
        communicator->SendN(counter, communicator->rankCoordinator(), PHASE_FINAL);
    }
}

void Algorithm::phaseFinalMatrix() {
    // Firstly, send results to the replication group leader.
    int divider = communicator->rank() / c;
    auto comm_replication = communicator->Split(divider);
    std::vector<std::unique_ptr<matrix::Dense>> ds;
    if (comm_replication.isCoordinator()) {
        ds.push_back(std::move(matrixC));
        for (int i = 1; i < comm_replication.numProcesses(); i++) {
            auto m = comm_replication.ReceiveDense(i, PHASE_FINAL);
            ds.push_back(std::move(m));
        }
    } else {
        comm_replication.SendDense(matrixC.get(), comm_replication.rankCoordinator(), PHASE_FINAL);
        return;
    }

    auto res = matrix::Merge(std::move(ds));
    ds.clear();

    // Secondly, send results to global coordinator.
    if (communicator->isCoordinator()) {
        ds.push_back(std::move(res));
        for (int i = c; i < communicator->numProcesses(); i += c) {
            auto m = communicator->ReceiveDense(i, PHASE_FINAL);
            ds.push_back(std::move(m));
        }
        auto final_result = matrix::Merge(std::move(ds));
        std::cout << *final_result << std::endl;
    } else {
        communicator->SendDense(res.get(), communicator->rankCoordinator(), PHASE_FINAL);
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

void AlgorithmCOLA::phaseComputationPartial() {
    auto it = matrix::SparseIt(matrixA.get());
    auto b_range = matrixB->ColumnRange();
    while (it.Next()) {
        auto itv = it.Value();

        int ay = std::get<0>(itv);
        int ax = std::get<1>(itv);

        assert(ax != -1);
        assert(ay != -1);

        double av = std::get<2>(itv);
        assert(av != 0);
        for (int bx = b_range.first; bx <= b_range.second; bx++) {
            double bv = matrixB->Get(bx, ax);
            auto cv = av * bv;
            matrixC->ItemAdd(bx, ay, cv);
        }
    }
}

void AlgorithmCOLA::phaseComputationCycleA(messaging::Communicator *comm) {
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

void AlgorithmCOLA::phaseComputation(int power) {
    auto comm_computation = communicator->Split(communicator->rank() % c);
    for (int p = 0; p < power; p++) {
        for (int i = 0; i < comm_computation.numProcesses(); i++) {
            phaseComputationPartial();
            phaseComputationCycleA(&comm_computation);
        }
        // Swap Matrix B with Matrix C.
        auto mb = std::move(matrixB);
        matrixB = std::move(matrixC);
        std::fill(mb->values.begin(), mb->values.end(), 0);
        matrixC = std::move(mb);
    }
    matrixC = std::move(matrixB);
}

}
