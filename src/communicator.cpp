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
    int meta[5] = {m->rows, m->column_base, m->columns, m->columns_total, static_cast<int>(m->values.size())};
    MPI_Send(&meta[0], 5, MPI_INT, receiver, phase, MPI_COMM_WORLD);
    MPI_Send(m->values.data(), m->values.size(), MPI_DOUBLE, receiver, phase, MPI_COMM_WORLD);
}

std::unique_ptr<matrix::Dense> Communicator::ReceiveDense(int sender, int phase) {
    int meta[5];
    MPI_Recv(&meta[0], 5, MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::vector<double> values(meta[4]);
    MPI_Recv(values.data(), values.size(), MPI_DOUBLE, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return std::make_unique<matrix::Dense>(meta[0], meta[1], meta[2], meta[3], std::move(values));
}

void Communicator::SendSparse(matrix::Sparse *m, int receiver, int phase) {
    int meta[3] = {static_cast<int>(m->values.size()), static_cast<int>(m->rows_number_of_values.size()), m->n};
    MPI_Send(&meta[0], 3, MPI_INT, receiver, phase, MPI_COMM_WORLD);
    MPI_Send(m->values.data(), m->values.size(), MPI_DOUBLE, receiver, phase, MPI_COMM_WORLD);
    MPI_Send(m->values_column.data(), m->values_column.size(), MPI_INT, receiver, phase, MPI_COMM_WORLD);
    MPI_Send(m->rows_number_of_values.data(), m->rows_number_of_values.size(), MPI_INT, receiver, phase, MPI_COMM_WORLD);
}

std::unique_ptr<matrix::Sparse> Communicator::ReceiveSparse(int sender, int phase) {
    int meta[3];
    MPI_Recv(&meta[0], 3, MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::vector<double> values(meta[0]);
    std::vector<int> values_column(meta[0]);
    std::vector<int> rows_number_of_values(meta[1]);
    MPI_Recv(values.data(), meta[0], MPI_DOUBLE, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(values_column.data(), meta[0], MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(rows_number_of_values.data(), meta[1], MPI_INT, sender, phase, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return std::make_unique<matrix::Sparse>(meta[2], std::move(values), std::move(rows_number_of_values),
        std::move(values_column));
}

}
