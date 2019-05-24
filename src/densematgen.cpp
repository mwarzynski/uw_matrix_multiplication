/*
 * Matrix generation library for the MPI assignment, HPC, Spring 2018/2019.
 *
 * Faculty of Mathematics, Informatics and Mechanics.
 * University of Warsaw, Warsaw, Poland.
 * 
 * The implementation is very, very loosely based on Xorshift RNGs, 
 * but doesn't keep the state, so it's not really pseudo-random.
 *
 * Krzysztof Rzadca
 * LGPL, 2019
 */
#include <cstdint>
#include "densematgen.h"


uint32_t naive_xorshift(uint32_t x, uint32_t y, uint32_t w) {
    x ^= x << 11;
    y ^= y << 7;
    x ^= y;
    w ^= w << 19;
    w ^= x;
    return w;
}


double generate_double(int seed, int row, int col) {
    if (seed == 0)
        return 0;
    if (seed == 1)
        return 1;
    if (seed == 2)
        return (row==col)? 1:0;
    if (seed == 3)
        return row*10+col;
    if (seed > 10)
    {

        uint32_t rand_32 = naive_xorshift((uint32_t) seed, (uint32_t) row,
                                          (uint32_t) col);
        uint32_t resolution = 1000;
        double rand = (rand_32 % resolution) / ((double) resolution);
        return (1.0 - 0.0) * rand;
    }
    return -1;
}
