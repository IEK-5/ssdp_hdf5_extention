#include <stdlib.h>
#include <math.h>
#include <hdf5.h>
#include <string.h>

#include "H5Enums.h"
#include "H5Dataset.h"
#include "H5Datatypes.h"
#include "H5Properties.h"

/*
    Create and initialize a H5DDatasetHandler struct.
    The struct must be freed!
    args:
        name: name of dataset should be like a unix path e.g. /foo/bar/datasetname
        loc: hid_t of the H5D file in which the dataset lives
    return:
        pointer to H5DatasetHandler struct
*/
struct H5DatasetHandler* H5DatasetHandler_init(const char *name, hid_t loc){
    struct H5DatasetHandler* self;
    self = malloc(sizeof(*self));
    if (NULL == self){
        return NULL;
    }
    self->loc = loc;
    self->name = strdup(name);
    self->read_ncols = -1;
    self->read_nrows = -1;
    self->read_data_columns = NULL;
    return self;
}


/*
    Write a column into a datset
    args:
        dataset_id: identifier of dataset to write to
        dataspace_id: identifier of dataspace belonging to dataset_id. It must be of shape nrows x ncols
        col_idx: index of column. 0 <= col_idx < ncols
        nrows: number of rows in the dataset
        column: array of length nrows
    return:
        0 if successful else a negative value
*/
static ErrorCode _write_column_to_dataset(hid_t dataset_id, hid_t dataspace_id, int col_idx, int nrows, double *column){
    ErrorCode out = SUCCESS;
    herr_t status = -1;
    hid_t hyperslab_dataspace_id = H5Screate_simple(2,(hsize_t []){nrows, 1},NULL);
    if (H5I_INVALID_HID == hyperslab_dataspace_id){
        return FAILURE;
    }
    const hsize_t start[2] = {0, col_idx};
    const hsize_t stride[2] = {1, 1};
    const hsize_t count[2] = {nrows, 1};
    const hsize_t block[2] = {1, 1};
    status = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, start, stride, count, block);
    if(status < 0){
        printf("Error creating hyperslab\n");
        out = FAILURE;
        goto error;
    }
    status  = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, hyperslab_dataspace_id, dataspace_id, H5P_DEFAULT, column);
    if(status < 0){
        out = FAILURE;
        printf("Error writing hyperslab\n");
        goto error;
    }
    status = H5Sselect_none(dataspace_id);
    if(status < 0){
        out = FAILURE;
        printf("Error undoing selection hyperslab\n");
        goto error;
    }
error:
    H5Sclose(hyperslab_dataspace_id);
    return out;
}


/**
   Write a 2D array of doubles which which is stored as an array of
   pointers to columns to a hd5 file in as a data set named
   dataset_name.

    args:
        self: Handler object created by H5DatasetHandler_init
        data: array of double * elements of data must point to contiguous memory
        nrows: rows of matrix, size of the contiguous arrays
        ncols: columns of matrix, size of the array of pointers
        disk_datatype: HDF5 Datatype to save the data on the disk
        chunk_size: number of rows making up a chunk for IO purposes
    return:
        SUCCESS if operation worked otherwise a FAILURE
*/
ErrorCode H5DatasetHandler_write_array_of_columns(struct H5DatasetHandler *self, double** data, int nrows, int ncols, hid_t disk_datatype, hsize_t chunk_size){
        /*
          TODOs:
          check if the values are in the correct range to fit into a 16 bit int
          add chunksize argument
        */
        hid_t dspace; // data space
        hid_t dset; // dataset
        hid_t dcp; // dataset create props
        hid_t dap; // dataset access props
        hid_t gcp; // create group props
        ErrorCode status = SUCCESS;
        hsize_t dims[2] = {nrows, ncols};
        const hsize_t chunk_dims[2] = {chunk_size,ncols};

        if ((dspace = H5Screate_simple(2,dims,NULL)) == H5I_INVALID_HID) {
                status = FAILURE;
                goto edspace;
        }

        if ((dcp = H5P_create_dataset_proplist(2, chunk_dims)) == H5I_INVALID_HID) {
                status = FAILURE;
                goto edcp;
        }

        if ((dap = H5P_create_16_MB_Chunk_Cache_access_dataset_proplist()) == H5I_INVALID_HID) {
                status = FAILURE;
                goto edap;
        }

        if ((gcp = H5P_create_group_proplist()) == H5I_INVALID_HID) {
                status = FAILURE;
                goto egcp;
        }


        if ((dset = H5Dcreate(self->loc, self->name, disk_datatype,
                              dspace, gcp, dcp, dap)) == H5I_INVALID_HID) {
                status = FAILURE;
                goto edset;
        }

        for(int col = 0; col < ncols; ++col) {
                status = _write_column_to_dataset(dset, dspace, col, nrows, data[col]);

                if (SUCCESS != status)
                        break;
        }

        H5Dclose(dset);
edset:
        H5Pclose(gcp);
egcp:
        H5Pclose(dap);
edap:
        H5Pclose(dcp);
edcp:
        H5Sclose(dspace);
edspace:
        return status;
}


/*
    Read a column from a dataset
    args:
        dataset_id: identifier of dataset to read
        dataspace_id: identifier of dataspace belonging to dataset_id. It must be of shape nrows x ncols
        col_idx: index of column. 0 <= col_idx < ncols
        out_column_address: address of a pointer to double under which the column will be stored
    return:
        0 if successful else a negative value

    This function allocates memory at *out_column_address which must be freed!
*/
static herr_t _read_column_from_dataset(hid_t dset, hid_t dspace, int col_idx, double **out_column_address){
        hid_t hslb; // hyperslab
        hsize_t dims[2], maxdims[2];

        // get the shape of the dataset
        // dims[0] = nrows
        H5Sget_simple_extent_dims(dspace, dims, maxdims);

        const hsize_t start[2] = {0, col_idx};
        const hsize_t stride[2] = {1, 1};
        const hsize_t count[2] = {dims[0], 1};
        const hsize_t block[2] = {1, 1};

        *out_column_address = malloc(dims[0]*sizeof(**out_column_address));
        if (NULL == *out_column_address)
                goto emalloc;

        if ((hslb = H5Screate_simple(2,(hsize_t []){dims[0], 1},NULL)) == H5I_INVALID_HID)
                goto ehslb;

        if (H5Sselect_hyperslab(dspace, H5S_SELECT_SET, start, stride, count, block) < 0)
                goto eselect;

        if (H5Dread(dset, H5T_NATIVE_DOUBLE, hslb, dspace, H5P_DEFAULT, *out_column_address) < 0)
                goto edread;

        if (H5Sselect_none(dspace) < 0)
                goto eselectnone;

        H5Sclose(hslb);
        return 0;
eselectnone:
edread:
eselect:
        H5Sclose(hslb);
ehslb:
        free(*out_column_address);
emalloc:
        return -1;
}


/**
    Read a 2D Dataset with fixed sizes from a HDF5 file into the
    Handler struct

    This function allocated memory in
    H5DatasetHandler->read_data_columns which needs to be freed!

    args:
        self: pointer returned by H5DatasetHandler_init
        maxncols: maximum number of columns to read
    return:
        SUCCESS if operation worked otherwise an enum with a nonzero value
*/
ErrorCode H5DatasetHandler_read_array_of_columns(struct H5DatasetHandler *self, int maxncols){
        ErrorCode status = SUCCESS;
        hid_t dspace, dset;
        hsize_t dims[2], maxdims[2];
        int col_alloc = 0;

        if ((dset = H5Dopen(self->loc, self->name, H5P_DEFAULT)) == H5I_INVALID_HID) {
                status = FAILURE;
                goto edset;
        }

        if((dspace = H5Dget_space(dset)) == H5I_INVALID_HID) {
                status = FAILURE;
                goto edspace;
        }

        // Find rank and retrieve current and maximum dimension sizes.
        if (H5Sget_simple_extent_dims(dspace, dims, maxdims) < 0) {
                status = FAILURE;
                goto endim;
        }

        self->read_nrows = dims[0];
        self->read_ncols = maxncols <= dims[1] ? maxncols : dims[1];

        self->read_data_columns = malloc(sizeof(*self->read_data_columns)*(self->read_ncols));
        if (NULL == self->read_data_columns){
                status = OUTOFMEMORY;
                goto emalloc;
        }

        for(col_alloc = 0; col_alloc < self->read_ncols; ++col_alloc)
                if (_read_column_from_dataset(dset, dspace, col_alloc, &(self->read_data_columns[col_alloc])) < 0) {
                        status = FAILURE;
                        break;
                }


    if (SUCCESS != status) {
            for (int i=0; i < col_alloc; ++i)
                    free(self->read_data_columns[i]);
            free(self->read_data_columns);
            self->read_data_columns = NULL;
            self->read_nrows = -1;
            self->read_ncols = -1;
    }
emalloc:
endim:
    H5Sclose(dspace);
edspace:
    H5Dclose(dset);
edset:
    return status;

}


/*
    free variables allocated by this struct then free the struct and set the pointer to NULL

    args:
        self_addr: address of pointer to the struct

    NOTE:
    self->read_data and self->read_data_columns are NOT freed! This is up to the user!
*/
void H5DatasetHandler_free(struct H5DatasetHandler **self_addr){
    struct H5DatasetHandler *self = *self_addr;
    free((char*)self->name);
    free(self);
    *self_addr = NULL;
}
