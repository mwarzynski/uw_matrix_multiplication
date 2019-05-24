#include <iostream>
#include <mpi.h>
#include "densematgen.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int num_processes, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int message;
    if (rank == 0) {
        message = 1337;
        MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&message, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << message << std::endl;
    }

    const int seed = 42;
    const int size = 10;
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            const double entry = generate_double(seed, r, c);
            std::cout << entry << " ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
