#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int num_processes, rank;

    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout << num_processes << std::endl;
    std::cout << rank << std::endl;

    MPI_Finalize();
    return 0;
}
