#include "matrixmul.h"

namespace matrixmul {

std::pair<int, int> group_divider(int rank, int replication_group_size, int processes) {
    int first_group_id = rank / replication_group_size;
    int first_group_item_id = rank % replication_group_size;
    int second_group_id = (first_group_id +
        first_group_item_id*(processes / (replication_group_size*replication_group_size))) %
            (processes / replication_group_size);
    return std::make_pair(first_group_id, second_group_id);
}

Algorithm::Algorithm(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com, int replication_factor,
    int seed, bool split_by_columns) {
    // Replicate Matrix A over the replication group.
    communicator = com;
    c = replication_factor;
    // Replicate Matrix A over the replication group.
    if (communicator->isCoordinator()) {
        n = full_matrix->rows;
        communicator->BroadcastSendN(n);
        auto matricesA = full_matrix->Split(communicator->numProcesses(), split_by_columns);
        matrixA = std::make_unique<matrix::Sparse>(matricesA[0]);
        for (size_t i = 1; i < matricesA.size(); i++) {
            communicator->SendSparse(&matricesA[i], i, PHASE_INITIALIZATION);
        }
    } else {
        n = communicator->BroadcastReceiveN();
        matrixA = communicator->ReceiveSparse(communicator->rankCoordinator(), PHASE_INITIALIZATION);
    }
    n_original = n;
    // Determine if we should expand n due to layers.
    if (n % replication_factor != 0) {
        n = ((n / replication_factor) + 1) * replication_factor;
    }
    // Prepare Matrix B and C.
    matrixB = std::make_unique<matrix::Dense>(n, n_original, communicator->rank(), communicator->numProcesses(), seed);
    matrixC = std::make_unique<matrix::Dense>(n, n_original, communicator->rank(), communicator->numProcesses());

    if (communicator->numProcesses() % replication_factor != 0) {
        throw std::runtime_error("p % c != 0");
    }
}

void Algorithm::phaseComputationPartial() {
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

        for (int bx = b_range.first; bx < b_range.second; bx++) {
            double bv = matrixB->Get(bx, ax);
            auto cv = av * bv;
            matrixC->ItemAdd(bx, ay, cv);
        }
    }
}

void Algorithm::phaseComputationCycleA(messaging::Communicator *comm) {
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

void Algorithm::phaseFinalGE(double g) {
    // Count how many values greater or equal to `g` is in the part of the result.
    long counter = 0;
    int i = 0;
    for (int row = 0; row < matrixC->n_original; row++) {
        for (int col = 0; col < matrixC->columns_total; col++) {
            if (matrixC->column_base <= col && col < matrixC->column_base + matrixC->columns) {
                if (col >= matrixC->n_original) {
                    i++;
                    continue;
                }
                if (matrixC->values[i++] >= g) {
                    counter += 1;
                }
            }
        }
    }
    // Send the counters to coordinator and print out the results.
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

void AlgorithmCOLA::phaseFinalMatrix() {
    // Divide the processes into replication groups.
    int divider = communicator->rank() / c;
    auto comm_replication = communicator->Split(divider);
    std::vector<std::unique_ptr<matrix::Dense>> matrices;
    // Firstly, send results to the replication group leader.
    if (comm_replication.isCoordinator()) {
        // Add process's own matrix to the result.
        matrices.push_back(std::move(matrixC));
        // Receive matrix results from other processes.
        for (int p = 1; p < comm_replication.numProcesses(); p++) {
            auto matrix = comm_replication.ReceiveDense(p, PHASE_FINAL);
            matrices.push_back(std::move(matrix));
        }
    } else {
        // If we aren't the coordinator in the replication group - just send the results and exit.
        // There is nothing more to do.
        comm_replication.SendDense(matrixC.get(), comm_replication.rankCoordinator(), PHASE_FINAL);
        return;
    }

    // Merge results received from replication groups.
    auto replication_result = matrix::Merge(std::move(matrices));
    // Free memory / prepare vector for next pushes. (Solely for reader's clarity - look up move above.)
    matrices.clear();

    // Secondly, send results to the global coordinator.
    if (communicator->isCoordinator()) {
        // Coordinator shouldn't send his own part to himself.
        // Therefore we just push the part to our result vector.
        matrices.push_back(std::move(replication_result));
        // Coordinator: receive parts from every other process.
        for (int i = c; i < communicator->numProcesses(); i += c) {
            auto m = communicator->ReceiveDense(i, PHASE_FINAL);
            matrices.push_back(std::move(m));
        }
        // Coordinator: all parts where received.
        // Coordinator: print out the Matrix.
        auto final_result = matrix::Merge(std::move(matrices));
        std::cout << final_result->n_original << " " << final_result->n_original << std::endl;
        std::cout << *final_result << std::endl;
    } else {
        // Not a coordinator: send managed part to the coordinator.
        communicator->SendDense(replication_result.get(), communicator->rankCoordinator(), PHASE_FINAL);
    }
}

AlgorithmCOLA::AlgorithmCOLA(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com,
    int replication_factor, int seed) : Algorithm(std::move(full_matrix), com, replication_factor, seed, true) { }

void AlgorithmCOLA::phaseReplication() {
    // Replicate Matrix A (this algorithm only replicates Matrix A).
    auto matrixA_copy = *matrixA;
    // Group processes which are next to each other together (012 345 678 ...).
    int divider = communicator->rank() / c;
    auto comm_replication = communicator->Split(divider);
    // At this point, `comm_replication` is a communicator used within replication group.
    for (int i = 0; i < comm_replication.numProcesses(); i++) {
        if (i == comm_replication.rank()) {
            comm_replication.BroadcastSendSparse(&matrixA_copy);
        } else {
            auto b = comm_replication.BroadcastReceiveSparse(i);
            matrixA = std::make_unique<matrix::Sparse>(matrixA.get(), b.get());
        }
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

AlgorithmInnerABC::AlgorithmInnerABC(std::unique_ptr<matrix::Sparse> full_matrix, messaging::Communicator *com,
                                 int replication_factor, int seed) :
                                 Algorithm(std::move(full_matrix), com, replication_factor, seed, false) {
    if (communicator->numProcesses() % (replication_factor*replication_factor) != 0) {
        throw std::runtime_error("p % c^2 != 0");
    }
}

void AlgorithmInnerABC::phaseReplication() {
    // Replicate A.
    auto matrixA_copy = *matrixA;
    auto divider = group_divider(communicator->rank(), c, communicator->numProcesses());
    auto comm_replication_a = communicator->Split(divider.second);
    // At this point, `comm_replication` is a communicator used within replication group.
    for (int i = 0; i < comm_replication_a.numProcesses(); i++) {
        if (i == comm_replication_a.rank()) {
            comm_replication_a.BroadcastSendSparse(&matrixA_copy);
        } else {
            auto b = comm_replication_a.BroadcastReceiveSparse(i);
            matrixA = std::make_unique<matrix::Sparse>(matrixA.get(), b.get());
        }
    }

    // Replicate B / C.
    auto comm_replication_b = communicator->Split(divider.first);
    matrix::Denses matrices_b;
    for (int i = 0; i < comm_replication_b.numProcesses(); i++) {
        if (i == comm_replication_b.rank()) {
            comm_replication_b.BroadcastSendDense(matrixB.get());
            matrices_b.push_back(std::move(matrixB));
        } else {
            auto b = comm_replication_b.BroadcastReceiveDense(i);
            matrices_b.push_back(std::move(b));
        }
    }
    matrixB = matrix::Merge(std::move(matrices_b));
    matrixC = std::make_unique<matrix::Dense>(matrixB->rows, n_original, matrixB->ColumnRange());
}

void AlgorithmInnerABC::phaseComputation(int power) {
    auto comm_replication_a = communicator->Split(communicator->rank() % c);
    int rounds = communicator->numProcesses() / (c*c);
    for (int i = 0; i < power; i++) {
        for (int j = 0; j < rounds; j++) {
            phaseComputationPartial();
            phaseComputationCycleA(&comm_replication_a);
        }
        // Swap Matrix B with Matrix C.
        auto mb = std::move(matrixB);
        matrixB = std::move(matrixC);
        std::fill(mb->values.begin(), mb->values.end(), 0);
        matrixC = std::move(mb);
    }
    matrixC = std::move(matrixB);
}

void AlgorithmInnerABC::phaseFinalMatrix() {
    // Divide the processes into replication groups.
    auto divider = group_divider(communicator->rank(), c, communicator->numProcesses());
    auto comm_replication = communicator->Split(divider.first);
    std::vector<std::unique_ptr<matrix::Dense>> matrices;
    // Firstly, send results to the replication group leader.
    if (comm_replication.isCoordinator()) {
        // Add process's own matrix to the result.
        matrices.push_back(std::move(matrixC));
        // Receive matrix results from other processes.
        for (int p = 1; p < comm_replication.numProcesses(); p++) {
            auto matrix = comm_replication.ReceiveDense(p, PHASE_FINAL);
            matrices.push_back(std::move(matrix));
        }
    } else {
        // If we aren't the coordinator in the replication group - just send the results and exit.
        // There is nothing more to do.
        comm_replication.SendDense(matrixC.get(), comm_replication.rankCoordinator(), PHASE_FINAL);
        return;
    }

    // Merge results received from replication groups.
    auto replication_result = matrix::Merge(std::move(matrices));
    // Free memory / prepare vector for next pushes. (Solely for reader's clarity - look up move above.)
    matrices.clear();

    // Secondly, send results to the global coordinator.
    if (communicator->isCoordinator()) {
        // Coordinator shouldn't send his own part to himself.
        // Therefore we just push the part to our result vector.
        matrices.push_back(std::move(replication_result));
        // Coordinator: receive parts from every other process.
        for (int i = c; i < communicator->numProcesses(); i += c) {
            auto m = communicator->ReceiveDense(i, PHASE_FINAL);
            matrices.push_back(std::move(m));
        }
        // Coordinator: all parts where received.
        // Coordinator: print out the Matrix.
        auto final_result = matrix::Merge(std::move(matrices));
        std::cout << final_result->n_original << " " << final_result->n_original << std::endl;
        std::cout << *final_result << std::endl;
    } else {
        // Not a coordinator: send managed part to the coordinator.
        communicator->SendDense(replication_result.get(), communicator->rankCoordinator(), PHASE_FINAL);
    }
}

}
