#include "utils.h"
#include <stdio.h>

void print_array(int *array, int length){
    for (int i = 0; i < length; i++){
        printf("%i ",array[i]);
    }
    printf("\n");
}