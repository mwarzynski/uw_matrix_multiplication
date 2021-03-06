#include "communicator.h"

namespace messaging {

Communicator::Communicator(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    _comm = MPI_COMM_WORLD;
    MPI_Comm_size(_comm, &_num_processes);
    MPI_Comm_rank(_comm, &_rank);
}

Communicator::Communicator(MPI_Comm base_comm, int base_rank, int divider) {
    MPI_Comm_split(base_comm, divider, base_rank, &_comm);
    MPI_Comm_size(_comm, &_num_processes);
    MPI_Comm_rank(_comm, &_rank);
}

Communicator::~Communicator() {
    if (_comm != MPI_COMM_WORLD) {
        MPI_Comm_free(&_comm);
    } else {
        MPI_Finalize();
    }
}

Communicator Communicator::Split(int divider) {
    return Communicator(_comm, _rank, divider);
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

void Communicator::BroadcastSendN(int n) {
    MPI_Bcast(&n, 1, MPI_INT, _rank, _comm);
}

int Communicator::BroadcastReceiveN() {
    int n;
    MPI_Bcast(&n, 1, MPI_INT, rankCoordinator(), _comm);
    return n;
}

void Communicator::SendN(long n, int receiver, int phase) {
    MPI_Send(&n, 1, MPI_LONG, receiver, phase, _comm);
}

long Communicator::ReceiveN(int sender, int phase) {
    long n;
    MPI_Recv(&n, 1, MPI_LONG, sender, phase, _comm, MPI_STATUS_IGNORE);
    return n;
}

void Communicator::SendDense(matrix::Dense *m, int receiver, int phase) {
    int meta[6] = {m->rows, m->column_base, m->columns, m->columns_total, static_cast<int>(m->values.size()), m->n_original};
    MPI_Send(&meta[0], 6, MPI_INT, receiver, phase, _comm);
    MPI_Send(m->values.data(), m->values.size(), MPI_DOUBLE, receiver, phase, _comm);
}

std::unique_ptr<matrix::Dense> Communicator::ReceiveDense(int sender, int phase) {
    int meta[6];
    MPI_Recv(&meta[0], 6, MPI_INT, sender, phase, _comm, MPI_STATUS_IGNORE);
    std::vector<double> values(meta[4]);
    MPI_Recv(values.data(), values.size(), MPI_DOUBLE, sender, phase, _comm, MPI_STATUS_IGNORE);
    return std::make_unique<matrix::Dense>(meta[0], meta[5], meta[1], meta[2], meta[3], std::move(values));
}

void Communicator::BroadcastSendDense(matrix::Dense *m) {
    int meta[6] = {m->rows, m->column_base, m->columns, m->columns_total, static_cast<int>(m->values.size()), m->n_original};
    MPI_Bcast(&meta[0], 6, MPI_INT, _rank, _comm);
    MPI_Bcast(m->values.data(), m->values.size(), MPI_DOUBLE, _rank, _comm);
}

std::unique_ptr<matrix::Dense> Communicator::BroadcastReceiveDense(int root) {
    int meta[6];
    MPI_Bcast(&meta[0], 6, MPI_INT, root, _comm);
    std::vector<double> values(meta[4]);
    MPI_Bcast(values.data(), values.size(), MPI_DOUBLE, root, _comm);
    return std::make_unique<matrix::Dense>(meta[0], meta[5], meta[1], meta[2], meta[3], std::move(values));
}

void Communicator::SendSparse(matrix::Sparse *m, int receiver, int phase) {
    int meta[3] = {static_cast<int>(m->values.size()), static_cast<int>(m->rows_number_of_values.size()), m->n};
    MPI_Send(&meta[0], 3, MPI_INT, receiver, phase, _comm);
    MPI_Send(m->values.data(), m->values.size(), MPI_DOUBLE, receiver, phase, _comm);
    MPI_Send(m->values_column.data(), m->values_column.size(), MPI_INT, receiver, phase, _comm);
    MPI_Send(m->rows_number_of_values.data(), m->rows_number_of_values.size(), MPI_INT, receiver, phase, _comm);
}

std::unique_ptr<matrix::Sparse> Communicator::ReceiveSparse(int sender, int phase) {
    int meta[3];
    MPI_Recv(&meta[0], 3, MPI_INT, sender, phase, _comm, MPI_STATUS_IGNORE);
    std::vector<double> values(meta[0]);
    std::vector<int> values_column(meta[0]);
    std::vector<int> rows_number_of_values(meta[1]);
    MPI_Recv(values.data(), meta[0], MPI_DOUBLE, sender, phase, _comm, MPI_STATUS_IGNORE);
    MPI_Recv(values_column.data(), meta[0], MPI_INT, sender, phase, _comm, MPI_STATUS_IGNORE);
    MPI_Recv(rows_number_of_values.data(), meta[1], MPI_INT, sender, phase, _comm, MPI_STATUS_IGNORE);
    return std::make_unique<matrix::Sparse>(meta[2], std::move(values), std::move(rows_number_of_values),
        std::move(values_column));
}

void Communicator::BroadcastSendSparse(matrix::Sparse *m) {
    int meta[3] = {static_cast<int>(m->values.size()), static_cast<int>(m->rows_number_of_values.size()), m->n};
    MPI_Bcast(&meta[0], 3, MPI_INT, _rank, _comm);
    MPI_Bcast(m->values.data(), m->values.size(), MPI_DOUBLE, _rank, _comm);
    MPI_Bcast(m->values_column.data(), m->values_column.size(), MPI_INT, _rank, _comm);
    MPI_Bcast(m->rows_number_of_values.data(), m->rows_number_of_values.size(), MPI_INT, _rank, _comm);
}

std::unique_ptr<matrix::Sparse> Communicator::BroadcastReceiveSparse(int root) {
    int meta[3];
    MPI_Bcast(&meta[0], 3, MPI_INT, root, _comm);
    std::vector<double> values(meta[0]);
    std::vector<int> values_column(meta[0]);
    std::vector<int> rows_number_of_values(meta[1]);
    MPI_Bcast(values.data(), meta[0], MPI_DOUBLE, root, _comm);
    MPI_Bcast(values_column.data(), meta[0], MPI_INT, root, _comm);
    MPI_Bcast(rows_number_of_values.data(), meta[1], MPI_INT, root, _comm);
    return std::make_unique<matrix::Sparse>(meta[2], std::move(values), std::move(rows_number_of_values),
                                            std::move(values_column));
}

}
