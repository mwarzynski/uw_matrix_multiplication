#include <iostream>
#include <mpi.h>
#include "densematgen.h"
#include "parse.h"

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int num_processes, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    Arguments *arguments;
    try {
        arguments = new Arguments(argc, argv);
    } catch (std::exception &e) {
        std::cout << "Arguments: " << e.what() << std::endl;
        MPI_Finalize();
        return 1;
    }

    // Implement me.

    MPI_Finalize();
    return 0;
}
