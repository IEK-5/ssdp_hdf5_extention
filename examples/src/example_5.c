#include <hdf5.h>
#include <stdlib.h>
#include <stdio.h>
/*
    In this example we create a Table 10.000 and 3 of 16 bit integers.
    We also create a Dataset of floats. It contains 10000 random uniform numbers.
    The Table has the column names with the A, B and C
    A has the sequence 1 0 1 0 1 ...
    B has the sequence 1 1 1 1 1 ...
    C has the sequence 1 2 3 4 5 ...
    We store the file once contiguously and once compressed.
*/

void create_sequence_a(int *array, int length){
    for(int i = 0; i < length; i++){
        array[i]=(i+1)%2;
    }
}

void create_sequence_b(int *array, int length){
    for(int i = 0; i < length; i++){
        array[i]=1;
    }
}

void create_sequence_c(int *array, int length){
    for(int i = 0; i < length; i++){
        array[i]=i+1;
    }
}

void create_sequence_d(int *array, int length){
    for(int i = 0; i < length; i++){
        array[i]=(i+1)*(i+1);
    }
}

float get_random_number(){
    /*
    It may not be a good random number generator.
    But it's a 2 liner.
    */
    float r = (float) rand();
    return r / (float) RAND_MAX;
}

int main(){
    srand(17);
     
}