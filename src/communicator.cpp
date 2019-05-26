#include "communicator.h"

namespace messaging {

Communicator::Communicator(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &_num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
}

Communicator::~Communicator() {
    MPI_Finalize();
}

bool Communicator::isCoordinator() {
    return _rank == Communicator::rankCoordinator();
}

int Communicator::rankCoordinator() {
    return 0;
}

int Communicator::rank() {
    return _rank;
}

int Communicator::numProcesses() {
    return _num_processes;
}

void Communicator::BroadcastN(int n) {
    MPI_Bcast(&n, 1, MPI_INT, rankCoordinator(), MPI_COMM_WORLD);
}

int Communicator::ReceiveN() {
    int n;
    MPI_Bcast(&n, 1, MPI_INT, rankCoordinator(), MPI_COMM_WORLD);
    return n;
}

void Communicator::SendDense(matrix::Dense *m, int receiver, int phase) {
    // Send meta data.
    int meta[5];
    meta[0] = m->rows;
    meta[1] = m->column_base;
    meta[2] = m->columns;
    meta[3] = m->columns_total;
    meta[4] = m->values.size();
    MPI_Send(&meta[0], 5, MPI_INT, receiver, phase, MPI_COMM_WORLD);
    // Send actual data.
    MPI_Send(m->values.data(), m->values.size(), MPI_DOUBLE, receiver, phase, MPI_COMM_WORLD);
}

std::unique_ptr<matrix::Dense> Communicator::ReceiveDense(int sender, int phase) {
    int meta[5];
    MPI_Recv(&meta[0], 5, MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    auto matrix = std::make_unique<matrix::Dense>();
    matrix->rows = meta[0];
    matrix->column_base = meta[1];
    matrix->columns = meta[2];
    matrix->columns_total = meta[3];
    matrix->values.resize(meta[4]);
    MPI_Recv(matrix->values.data(), matrix->values.size(), MPI_DOUBLE, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return matrix;
}

void Communicator::SendSparse(matrix::Sparse *m, int receiver, int phase) {
    // Send meta data.
    int meta[3];
    meta[0] = m->values.size();
    meta[1] = m->rows_number_of_values.size();
    meta[2] = m->n;
    MPI_Send(&meta[0], 3, MPI_INT, receiver, phase, MPI_COMM_WORLD);
    // Send actual data.
    MPI_Send(m->values.data(), m->values.size(), MPI_DOUBLE, receiver, phase, MPI_COMM_WORLD);
    MPI_Send(m->values_column.data(), m->values_column.size(), MPI_INT, receiver, phase, MPI_COMM_WORLD);
    MPI_Send(m->rows_number_of_values.data(), m->rows_number_of_values.size(), MPI_INT, receiver, phase, MPI_COMM_WORLD);
}

std::unique_ptr<matrix::Sparse> Communicator::ReceiveSparse(int sender, int phase) {
    // Receive metadata.
    int meta[3];
    MPI_Recv(&meta[0], 3, MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    auto matrix = std::make_unique<matrix::Sparse>();
    matrix->values.resize(meta[0]);
    matrix->values_column.resize(meta[0]);
    matrix->rows_number_of_values.resize(meta[1]);
    matrix->n = meta[2];
    // Receive vector data.
    MPI_Recv(matrix->values.data(), meta[0], MPI_DOUBLE, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(matrix->values_column.data(), meta[0], MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(matrix->rows_number_of_values.data(), meta[1], MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return matrix;
}

}
