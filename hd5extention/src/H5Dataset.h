#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <hdf5.h>
#include "H5Enums.h"

/*
    This struct holds the necessary infos needed to
    read and write HDF5 Datasets

    attributes:
        read_nrows: contains the number of rows of the dataset if it had been read
        read_ncols: contains the number of columns of the dataset if it had been read
        read_data_columns: if read_array_of_columns is called this is an array of pointers to columns read from the dataset
        name: name of the dataset as a path within the HDF5 File e.g. dir1/subdir2/Dataset1
        loc: id of the of the group to which the dataset belongs.

*/
struct H5DatasetHandler{
    hsize_t read_nrows;
    hsize_t read_ncols;
    double **read_data_columns;
    const char *name;
    hid_t loc;
};

/*
    Create and initialize a H5DDatasetHandler struct.
    The struct must be freed using H5DatasetHandler_free!
    args:
        name: name of dataset should be like a unix path e.g. /foo/bar/datasetname
        loc: hid_t of the H5D file in which the dataset lives
    return:
        pointer to H5DatasetHandler struct
*/
struct H5DatasetHandler* H5DatasetHandler_init(const char *name, hid_t loc);

/*
    Write a 2D array of doubles which which is stored as an array of pointers to columns to a hd5 file in as a data set named dataset_name.
    args:
        self: Handler object created by H5DatasetHandler_init
        data: array of double * elements of data must point to contiguous memory
        nrows: rows of matrix, size of the contiguous arrays
        ncols: columns of matrix, size of the array of pointers
        disk_datatype: HDF5 Datatype to save the data on the disk
        chunk_size: number of rows making up a chunk for IO purposes
    return:
        SUCCESS if operation worked otherwise an enum with a nonzero value
*/ 
ErrorCode H5DatasetHandler_write_array_of_columns(struct H5DatasetHandler *self, double** data, int nrows, int ncols, hid_t disk_datatype, hsize_t chunk_size);


/*
    Read a 2D Dataset with fixed sizes from a HDF5 file into the Handler struct
    This function allocates memory in H5DatasetHandler->read_data_columns which needs to be freed!
    args:
        self: pointer returned by H5DatasetHandler_init
        maxncols: maximum number of columns to read
    return:
        SUCCESS if operation worked otherwise an enum with a nonzero value
*/
ErrorCode H5DatasetHandler_read_array_of_columns(struct H5DatasetHandler *self, hsize_t maxncols);

/*
 * Free a handler created by H5DatasetHandler_init
 */
void H5DatasetHandler_free(struct H5DatasetHandler **self_addr);

