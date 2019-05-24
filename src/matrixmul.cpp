#include "matrixmul.h"

MatrixCRS::MatrixCRS(int n, std::vector<double> &&nonzero_values, std::vector<int> &&extents_of_rows,
    std::vector<int> &&column_indices) : n{n}, nonzero_values{nonzero_values}, extents_of_rows{extents_of_rows},
    column_indices{column_indices} {}
