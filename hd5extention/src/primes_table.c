#include "primes_table.h"
#include "_primes.h"

unsigned int get_nth_prime(unsigned int index){
    if(index >= N_PRIMES){
        return 0;
    }
    return PRIMES[index];
}
