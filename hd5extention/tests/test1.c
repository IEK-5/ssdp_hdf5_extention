#include <hdf5.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "enums.h"
#include "H5FileIO.h"

int main(){
    int nrows = 5;
    int ncols = 3;
    int k = 0;
    double *data = malloc(sizeof(double)*nrows*ncols);
    const char *filename = "file4_pretty.h5";
    const char *datasetname = "some_data";
    double *read_data;
    int read_nrows;
    int read_ncols;
    struct H5FileIOHandler* file;
    ErrorCode err;

    for(int i = 0; i<nrows; i++){
        for(int j = 0; j<ncols; j++){
            data[i*ncols+j]=(k++);
        }
    }
    
    file = H5FileIOHandler_init(filename, CREATE);
    err = H5FileIOHandler_write_array(file, datasetname, data, nrows, ncols);
    printf("The writing of the data was: %s\n", ErrorCode_to_string(err));
    H5FileIOHandler_free(file);
    
    file =  H5FileIOHandler_init(filename, READ);
    err = H5FileIOHander_read_array(file, datasetname, &read_data, &read_nrows, &read_ncols);
    printf("The reading of the data was: %s\n", ErrorCode_to_string(err));
    H5FileIOHandler_free(file);

    bool allsame = true;
    const char * answer;
    for(int i = 0; i<nrows; i++){
        for(int j = 0; j<ncols; j++){
            allsame &= data[i*ncols+j]==read_data[i*ncols+j];
        }
    }
    
    printf("Are the written and read data the same? %s \n", allsame ? "yes" : "no");
    free(data);
    free(read_data);
    
    return 0;
}