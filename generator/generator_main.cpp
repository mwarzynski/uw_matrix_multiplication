/*
 * Example usage for the matrix generation library, HPC, Spring 2018/2019.
 *
 * Faculty of Mathematics, Informatics and Mechanics.
 * University of Warsaw, Warsaw, Poland.
 * 
 * Krzysztof Rzadca
 * LGPL, 2019
 */

#include <iostream>
#include "densematgen.h"


int main() {
    const int seed = 42;
    const int size = 10;
    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            const double entry = generate_double(seed, r, c);
            std::cout << entry << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}

