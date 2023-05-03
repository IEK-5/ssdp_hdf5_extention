#include <hdf5.h>
#include <H5public.h>
#include <stdio.h>
#include <stdint.h>
// #include <criterion/criterion.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>

 #include <ctype.h>
 #include <unistd.h>


#include "H5Datatypes.h"
#include "valgrind/callgrind.h"

#include "H5Enums.h"
#include "utils.h"
#include "H5FileIO.h"

double * make_zeros(int nrows, int ncols){
    double * data = calloc(nrows*ncols, sizeof(double));
    return data;
}
/*
Test(WriteCompressedTables,WriteZerosDifferentChunkSizes){
    int nrows = 10000;
    int ncols = 5;
    double* data = make_zeros(nrows, ncols);
    const char ** columns_names = malloc(sizeof(char *)*nrows);
    columns_names[0] = "A";
    columns_names[1] = "B";
    columns_names[2] = "C";
    columns_names[3] = "D";
    columns_names[4] = "F";

    recursive_delete("compressed_files/tables");
    make_dir("compressed_files/tables");
    const char* template = "compressed_files/tables/chunksize_%d.h5";
    char *filename = malloc(sizeof(char)*100);
    struct H5FileIOHandler* handler;
    ErrorCode err;
    int status;
    for(hsize_t chunk_size = 1; chunk_size < nrows; chunk_size*=10){
        status = snprintf(filename, 100, template, chunk_size);
        cr_assert(status > 0);
        cr_assert(status < 100);
        handler = H5FileIOHandler_init(filename, W);
        err = H5FileIOHandler_write_table(handler, "data", data , nrows, ncols, columns_names, chunk_size);
        cr_assert(SUCCESS == err);
        H5FileIOHandler_free(&handler);
    }
}
*/

void write_dataset(char *filename, double *data, int nrows, int ncols, size_t chunk_size){
    ErrorCode err;
    herr_t status;
    struct H5FileIOHandler* handler;
    handler = H5FileIOHandler_init(filename, W);
    hid_t small_float = H5T_define_16bit_float();
    err = H5FileIOHandler_write_array(handler, "data", data, nrows, ncols, chunk_size, small_float);
    //cr_assert(SUCCESS == err);
    status = H5Tclose(small_float);
    //cr_assert(status >= 0);
    H5FileIOHandler_free(&handler);
}

//Test(WriteCompressedDatasets,WriteZerosDifferentChunkSizes){
//int main(){
void create_dataset(hsize_t chunk_size){
    int nrows = 26843545;
    int ncols = 5;
    double* data = make_zeros(nrows, ncols);
    
    //recursive_delete("compressed_files/datasets");
    make_dir("compressed_files/datasets");
    const char* template = "compressed_files/datasets/chunksize_%d.h5";
    char *filename = malloc(sizeof(char)*100);
    
    
    int status;
    status = snprintf(filename, 100, template, chunk_size);
        //cr_assert(status > 0);
        //cr_assert(status < 100);
        CALLGRIND_START_INSTRUMENTATION;
        CALLGRIND_TOGGLE_COLLECT;
        write_dataset(filename, data, nrows, ncols, chunk_size);
        CALLGRIND_TOGGLE_COLLECT;
        CALLGRIND_STOP_INSTRUMENTATION;
    free(data);
    //return 0;
}


int main(int argc, char* argv[]){
    size_t chunk_size;
    if (argc == 1){
        printf("Provide a chunk size as a cli argument!\n");
        exit(1);
    }
    char *chunk_size_str = argv[1];
    if(sscanf(chunk_size_str,"%lu", &chunk_size) == 0){
        printf("Error reading chunk size!\n");
        exit(1);
    }
    printf("Creating file with chunk size: %lu\n", chunk_size);
    create_dataset(chunk_size);
    

}