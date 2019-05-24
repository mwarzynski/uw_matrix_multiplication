#include "parser.h"


namespace parser {

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

MatrixCRS* parse_sparse_matrix(const std::string &filename) {
    int rows, columns, total_items, max_row_items;
    std::vector<double> nonzero_values;
    std::vector<int> extents_of_rows;
    std::vector<int> column_indices;

    std::ifstream f;
    f.open(filename);

    try {
        f >> rows >> columns >> total_items >> max_row_items;

        if (rows != columns) {
            throw std::invalid_argument("Matrix hasn't square dimensions.");
        }
        if (total_items < 0) {
            throw std::invalid_argument("Matrix total number of non-zero items is negative.");
        }
        if (max_row_items < 0) {
            throw std::invalid_argument("Matrix number of max row items is negative.");
        }

        double value;
        for (int i = 0; i < total_items; i++) {
            f >> value;
            nonzero_values.push_back(value);
        }
        int item;
        for (int i = 0; i <= total_items; i += max_row_items) {
            f >> item;
            extents_of_rows.push_back(item);
        }
        for (int i = 0; i < total_items; i++) {
            f >> item;
            column_indices.push_back(item);
        }
    } catch (std::exception &e) {
        f.close();
        std::rethrow_exception(std::current_exception());
    }
    f.close();
    return new MatrixCRS(rows, std::move(nonzero_values), std::move(extents_of_rows),
        std::move(column_indices));
}

}
