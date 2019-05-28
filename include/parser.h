#ifndef UW_MATRIX_MULTIPLICATION_PARSER_H
#define UW_MATRIX_MULTIPLICATION_PARSER_H

#include <memory>
#include <fstream>
#include <string>
#include <stdexcept>
#include <getopt.h>
#include "matrixmul.h"


namespace parser {

class Arguments {
public:

    std::string sparse_matrix_file;
    int seed = 0;
    matrixmul::Algorithms algorithm = matrixmul::COLA;
    bool print_the_matrix_c = false;
    int replication_group_size = 1;
    int exponent = 0;
    double ge_value = 0;
    bool mkl = false;

    Arguments(int argc, char **argv);
};

std::unique_ptr<matrix::Sparse> parse_sparse_matrix(const std::string &filename);

}

#endif //UW_MATRIX_MULTIPLICATION_PARSER_H
