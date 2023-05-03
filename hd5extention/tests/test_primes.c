#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include "limits.h"
#include "primes_table.h"

Test(TestPrimes, valid_index){
    unsigned int indices[5] = {0, 17, 420, 8008, N_PRIMES-1};
    int expected[5] = {2, 61, 2909, 81919, 99991};
    for(int i = 0; i < 5; i++){
        cr_assert(eq(uint, get_nth_prime(indices[i]), expected[i]));
    }
}

Test(TestPrimes, invalid_index){
    unsigned int indices[3] = {-1, N_PRIMES, UINT_MAX};
    for(int i = 0; i < 3; i++){
        cr_assert(eq(uint, get_nth_prime(indices[i]), 0));
    }
}