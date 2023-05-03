#pragma once
#include <stddef.h>

extern unsigned const int PRIMES[];
extern const size_t N_PRIMES;
/*
    get the n-th prime number

    args:
        index: index of the prime e.g.
        prime: 2,3,5,7
        index: 0,1,2,3
    return:
        prime if successfull else 0

    This function reads from a table therefor it may fail.
*/
unsigned int get_nth_prime(unsigned int index);

