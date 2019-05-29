#ifndef UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
#define UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H

#include <memory>
#include "mpi.h"
#include "matrix.h"


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
    int rankCoordinator();
    int rank();
    int numProcesses();

    void BroadcastSendN(int n);
    int BroadcastReceiveN();

    void SendN(long n, int receiver, int phase);
    long ReceiveN(int sender, int phase);

    void SendDense(matrix::Dense *m, int receiver, int phase);
    std::unique_ptr<matrix::Dense> ReceiveDense(int sender, int phase);

    void SendSparse(matrix::Sparse *m, int receiver, int phase);
    std::unique_ptr<matrix::Sparse> ReceiveSparse(int sender, int phase);

    void BroadcastSendSparse(matrix::Sparse *m);
    std::unique_ptr<matrix::Sparse> BroadcastReceiveSparse(int root);
};

}

#endif //UW_MATRIX_MULTIPLICATION_COMMUNICATOR_H
