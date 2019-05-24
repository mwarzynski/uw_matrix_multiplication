#include "matrixmul.h"

MatrixCRS::MatrixCRS(int n, std::vector<double> &&nonzero_values, std::vector<int> &&extents_of_rows,
    std::vector<int> &&column_indices) : n{n}, nonzero_values{nonzero_values}, extents_of_rows{extents_of_rows},
    column_indices{column_indices} {}

std::ostream& operator<<(std::ostream &os, const MatrixCRS &m) {
    int n = 0;
    for (int r = 0; r < m.n; r++) {
        int values_in_row = m.extents_of_rows[r+1] - m.extents_of_rows[r];
        for (int i = 0; i < m.n; i++) {
            if (values_in_row > 0 && i == m.column_indices[n]) {
                os << m.nonzero_values[n] << "\t";
                n++;
                values_in_row--;
            } else {
                os << "0.00000" << "\t";
            }
        }
        os << std::endl;
    }
    return os;
}
