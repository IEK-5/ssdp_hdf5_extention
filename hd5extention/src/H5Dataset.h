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
        name: name of the dataset as a path within the HDF5 File e.g. dir1/subdir2/Dataset1
        loc: id of the of the group to which the dataset belongs.

*/
struct H5DatasetHandler{
    hsize_t read_nrows;
    hsize_t read_ncols;
    double *read_data;
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
    Write a continuous 1d array of doubles which is interpreted as a matrix into a HDF5 file using the H5DatasetHandler helper struct
    args:
        self: Handler object created by H5DatasetHandler_init
        data: pointer to array of doubles
        nrows: rows of matrix
        ncols: columns of matrix
        chunk_size: number of rows making up a chunk for IO purposes
    return:
        SUCESS if operation worked otherwise an enum with a nonzero value
*/ 
ErrorCode H5DatasetHandler_write_array(struct H5DatasetHandler *self, double* data, int nrows, int ncols, hid_t disk_datatype, hsize_t chunk_size);

/*
    Read a 2D Dataset with fixed sizes from a HDF5 file into the Handler struct
    This function allocated memory in H5DatasetHandler->read_data which needs to be freed!
    args:
        self: pointer returned by H5DatasetHandler_init
    return:
        SUCESS if operation worked otherwise an enum with a nonzero value
*/
ErrorCode H5DatasetHandler_read_array(struct H5DatasetHandler *self);

/*
 * Free a handler created by H5DatasetHandler_init
 */
void H5DatasetHandler_free(struct H5DatasetHandler **self_addr);

