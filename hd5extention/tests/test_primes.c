#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include "limits.h"
#include "primes_table.h"

// THESE TESTS MAY FAIL IF YOU CHOOSE A _primes.h 
// file with less primes than the one provided with the sourcecode

Test(TestPrimes, get_nth_prime_valid_index){
    unsigned int indices[5] = {0, 17, 420, 8008, N_PRIMES-1};
    int expected[5] = {2, 61, 2909, 81919, 99991};
    for(int i = 0; i < 5; i++){
        cr_assert(eq(uint, get_nth_prime(indices[i]), expected[i]));
    }
}

Test(TestPrimes, get_nth_prime_invalid_index){
    unsigned int indices[3] = {-1, N_PRIMES, UINT_MAX};
    for(int i = 0; i < 3; i++){
        cr_assert(eq(uint, get_nth_prime(indices[i]), 0));
    }
}

Test(TestPrimes, test_next_biggest_prime){
    unsigned int values[] = {0, 1, 2, 4, PRIMES[N_PRIMES-2], PRIMES[N_PRIMES-1], PRIMES[N_PRIMES-1]+1};
    unsigned int expected[] = {2, 2, 3, 5, PRIMES[N_PRIMES-1], 0, 0};
    int n = sizeof(values) / sizeof(values[0]);
    for(int i = 0; i < n; i++){
        cr_assert(eq(uint, get_next_biggest_prime(values[i]), expected[i]),"index=%d\n",i);
    }
}