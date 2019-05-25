#include "matrixmul.h"

MatrixDense::MatrixDense(int n, int part, int parts_total, int seed) : rows{n} {
    this->columns = n / parts_total;
    this->column_base = columns * part;
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < columns; c++) {
            this->values.push_back(generate_double(seed, r, column_base + c));
        }
    }
}

std::ostream& operator<<(std::ostream &os, const MatrixDense &m) {
    int i = 0;
    for (int r = 0; r < m.rows; r++) {
        for (int c = 0; c < m.columns; c++) {
            os << "(" << r << "," << m.column_base + c << ")=";
            if (m.values[i] != 0) {
                os << m.values[i];
            } else {
                os << "0.000";
            }
            os << "\t";
            i++;
        }
        os << std::endl;
    }
    return os;
}

MatrixSparse::MatrixSparse(int n, std::vector<double> &&nonzero_values, std::vector<int> &&extents_of_rows,
    std::vector<int> &&column_indices) : n{n}, nonzero_values{nonzero_values}, extents_of_rows{extents_of_rows},
    column_indices{column_indices} {}

std::ostream& operator<<(std::ostream &os, const MatrixSparse &m) {
    int n = 0;
    for (int r = 0; r < m.n; r++) {
        int values_in_row = m.extents_of_rows[r+1] - m.extents_of_rows[r];
        for (int i = 0; i < m.n; i++) {
            if (values_in_row > 0 && i == m.column_indices[n]) {
                os << m.nonzero_values[n];
                n++;
                values_in_row--;
            } else {
                os << "0.00000";
            }
            os << "\t";
        }
        os << std::endl;
    }
    return os;
}
