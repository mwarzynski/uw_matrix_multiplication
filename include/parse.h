#ifndef UW_MATRIX_MULTIPLICATION_PARSE_H
#define UW_MATRIX_MULTIPLICATION_PARSE_H

#include <string>
#include <stdexcept>
#include <getopt.h>


enum Algorithms {
    COLA,   // 1.5D blocked column replicating A (ColA)
    COLABC, // 1.5D blocked column replicating all matrices (ColABC)
};

class Arguments {
public:

    std::string sparse_matrix_file;
    int seed_for_dens_matrix = 0;
    Algorithms algorithm = COLA;
    bool print_the_matrix_c = false;
    int replication_group_size = 0;
    int exponent = 0;
    int ge_value = 0;
    bool mkl = false;

    Arguments(int argc, char **argv);
};

#endif //UW_MATRIX_MULTIPLICATION_PARSE_H
