#ifndef UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
#define UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H

#include <memory>
#include "mpi.h"
#include "matrix.h"

#define PHASE_INITIALIZATION 10


namespace messaging {

class Communicator {
private:
    MPI_Comm _comm;
    int _rank;
    int _num_processes;
public:

    Communicator(int argc, char **argv);
    Communicator(MPI_Comm base_comm, int base_rank, int divider);
    ~Communicator();

    Communicator Split(int divider);

    bool isCoordinator();
    static int rankCoordinator();
    int rank();
    int numProcesses();

    void BroadcastN(int n);
    int ReceiveN();

    void SendDense(matrix::Dense *m, int receiver, int phase);
    std::unique_ptr<matrix::Dense> ReceiveDense(int sender, int phase);

    void SendSparse(matrix::Sparse *m, int receiver, int phase);
    std::unique_ptr<matrix::Sparse> ReceiveSparse(int sender, int phase);
};

}

#endif //UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
