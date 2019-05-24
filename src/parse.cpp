#include "parse.h"

Arguments::Arguments(int argc, char **argv) {
    int c;
    char *end;
    while ((c = getopt(argc, argv, "f:s:c:e:g:vim")) != -1) {
        switch (c) {
            case 'f':
                this->sparse_matrix_file = std::string(optarg);
                break;
            case 's':
                this->seed_for_dens_matrix = std::strtol(optarg, &end, 10);
                break;
            case 'c':
                this->replication_group_size = std::strtol(optarg, &end, 10);
                break;
            case 'e':
                this->exponent = std::strtol(optarg, &end, 10);
                break;
            case 'g':
                this->ge_value = std::strtol(optarg, &end, 10);
                break;
            case 'v':
                this->print_the_matrix_c = true;
                break;
            case 'i':
                this->algorithm = COLABC;
                break;
            case 'm':
                this->mkl = true;
                break;
            case '?':
                throw std::invalid_argument(std::string(1, optopt));
            default:
                throw std::runtime_error("Parsing arguments.");
        }
    }
    if (this->sparse_matrix_file.empty()) {
        throw std::invalid_argument("-f (sparse_matrix_file) is required.");
    }
    if (this->seed_for_dens_matrix <= 0) {
        throw std::invalid_argument("-s (seed_for_dense_matrix) is required and must be > 0.");
    }
    if (this->replication_group_size <= 0) {
        throw std::invalid_argument("-c (replication_group_size) is required and must be > 0.");
    }
    if (this->exponent <= 0) {
        throw std::invalid_argument("-e (exponent) is required and must be > 0.");
    }
}
