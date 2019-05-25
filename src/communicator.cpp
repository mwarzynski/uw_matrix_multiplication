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
    return _rank == 0;
}

int Communicator::rank() {
    return _rank;
}

int Communicator::numProcesses() {
    return _num_processes;
}

void Communicator::BroadcastMatrixN(int *n) {
    MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

}
