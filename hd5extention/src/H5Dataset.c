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
    self->read_data = NULL;
    self->read_ncols = -1;
    self->read_nrows = -1;
    self->read_data_columns = NULL;
    return self;
}

/*
    Write a continuous 1d array of doubles which is interpreted as a matrix into a HDF5 file using the H5DatasetHandler helper struct
    args:
        self: Handler object created by H5DatasetHandler_init
        data: pointer to contiguous array of doubles
        nrows: rows of matrix
        ncols: columns of matrix
        disk_datatype: HDF5 Datatype to save the data on the disk
        chunk_size: number of rows making up a chunk for IO purposes
    return:
        SUCCESS if operation worked otherwise an enum with a nonzero value
*/ 
ErrorCode H5DatasetHandler_write_array(struct H5DatasetHandler *self, double* data, int nrows, int ncols, hid_t disk_datatype, hsize_t chunk_size){
    /*
        TODOs:
            check if the values are in the correct range to fit into a 16 bit int
            add chunksize argument 
    */
    hsize_t dims[2];
    herr_t status;
    dims[0]=nrows;
    dims[1]=ncols;
    hid_t dataspace_id, dataset_id;
    hid_t dataspace_create_props, dataspace_access_props, group_create_props;
    dataspace_id = H5Screate_simple(2,dims,NULL);
    if(H5I_INVALID_HID == dataspace_id){
        return FAILURE;
    }
    const hsize_t chunk_dims[2] = {chunk_size,ncols};
    dataspace_create_props = H5P_create_dataset_proplist(2, chunk_dims);
    dataspace_access_props = H5P_create_16_MB_Chunk_Cache_access_dataset_proplist();
    group_create_props = H5P_create_group_proplist();
    dataset_id = H5Dcreate(self->loc, self->name, disk_datatype, dataspace_id, group_create_props, dataspace_create_props, dataspace_access_props);
    if (H5I_INVALID_HID == dataset_id){
        H5Sclose(dataspace_id);
        H5Pclose(dataspace_create_props);
        return FAILURE;
    }
    // TODO
    // https://docs.hdfgroup.org/hdf5/develop/group___h5_d.html#ga98f44998b67587662af8b0d8a0a75906
    // we should change this function or make a new function to be able to write partial/loose arrays into a single dataset
    status  = H5Dwrite(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

    H5Dclose(dataset_id);
    H5Pclose(group_create_props);
    H5Pclose(dataspace_access_props);
    H5Pclose(dataspace_create_props);
    H5Sclose(dataspace_id);

    if (0 > status){
        return FAILURE;
    }
    else{
        return SUCCESS;
    }
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
ErrorCode H5DatasetHandler_write_array_of_columns(struct H5DatasetHandler *self, double** data, int nrows, int ncols, hid_t disk_datatype, hsize_t chunk_size){
    /*
        TODOs:
            check if the values are in the correct range to fit into a 16 bit int
            add chunksize argument 
    */
    hsize_t dims[2];
    ErrorCode status = SUCCESS;
    dims[0]=nrows;
    dims[1]=ncols;
    hid_t dataspace_id, dataset_id;
    hid_t dataspace_create_props, dataspace_access_props, group_create_props;
    dataspace_id = H5Screate_simple(2,dims,NULL);
    if(H5I_INVALID_HID == dataspace_id){
        return FAILURE;
    }
    const hsize_t chunk_dims[2] = {chunk_size,ncols};
    dataspace_create_props = H5P_create_dataset_proplist(2, chunk_dims);
    dataspace_access_props = H5P_create_16_MB_Chunk_Cache_access_dataset_proplist();
    group_create_props = H5P_create_group_proplist();
    dataset_id = H5Dcreate(self->loc, self->name, disk_datatype, dataspace_id, group_create_props, dataspace_create_props, dataspace_access_props);
    if (H5I_INVALID_HID == dataset_id){
        H5Sclose(dataspace_id);
        H5Pclose(dataspace_create_props);
        return FAILURE;
    }
    
    for(int col = 0; col < ncols; col++){
        status = _write_column_to_dataset(dataset_id, dataspace_id, col,  nrows,  data[col]);
    }   

    H5Dclose(dataset_id);
    H5Pclose(group_create_props);
    H5Pclose(dataspace_access_props);
    H5Pclose(dataspace_create_props);
    H5Sclose(dataspace_id);

    return status;
}

/*
    Read a 2D Dataset with fixed sizes from a HDF5 file into the Handler struct
    This function allocated memory in H5DatasetHandler->read_data which needs to be freed!
    args:
        self: pointer returned by H5DatasetHandler_init
    return:
        SUCCESS if operation worked otherwise an enum with a nonzero value
*/
ErrorCode H5DatasetHandler_read_array(struct H5DatasetHandler *self){
    ErrorCode return_val = SUCCESS;
    hid_t dataspace_id, dataset_id;
    herr_t status;
    hsize_t dims[2];
    hsize_t maxdims[2];
    int n_dims;

    dataset_id = H5Dopen(self->loc, self->name, H5P_DEFAULT);
    if (H5I_INVALID_HID == dataset_id){
        return_val = FAILURE;
        goto error;
    }
    dataspace_id = H5Dget_space(dataset_id);
    if(H5I_INVALID_HID == dataspace_id){
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }
   
    // Find rank and retrieve current and maximum dimension sizes.
    n_dims = H5Sget_simple_extent_dims(dataspace_id, dims, maxdims);
    if (n_dims < 0){
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }

    self->read_nrows = dims[0];
    self->read_ncols = dims[1];
    
    self->read_data = malloc(sizeof(double)*dims[0]*dims[1]);
    
    if (NULL == self->read_data){
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = OUTOFMEMORY;
        goto error;
    }

    
    status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, self->read_data);

    if(status < 0){
        free(self->read_data);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }

    H5Sclose(dataspace_id);
    H5Dclose(dataset_id);
    return return_val;

error:
    self->read_data = NULL;
    self->read_nrows = -1;
    self->read_ncols = -1;
    return return_val;

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
static herr_t _read_column_from_dataset(hid_t dataset_id, hid_t dataspace_id, int col_idx, double **out_column_address){
    hsize_t dims[2];
    hsize_t maxdims[2];
    herr_t status = -1;
    
    // I know I do this in read_array_of_columns but technically read
    // does not need to know the extend explicitly and I want to avoid
    // programmer errors

    // get the shape of the dataset
    H5Sget_simple_extent_dims(dataspace_id, dims, maxdims);
    hsize_t nrows = dims[0];

    // allocate memory for the column
    *out_column_address = malloc(nrows*sizeof(double));
    if (NULL == *out_column_address){
        return -1;
    }
    // select a hyperslab
    hid_t hyperslab_dataspace_id = H5Screate_simple(2,(hsize_t []){nrows, 1},NULL);
    if (H5I_INVALID_HID == hyperslab_dataspace_id){
        return -1;
    }
    const hsize_t start[2] = {0, col_idx};
    const hsize_t stride[2] = {1, 1};
    const hsize_t count[2] = {nrows, 1};
    const hsize_t block[2] = {1, 1};
    status = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, start, stride, count, block);
    if(status < 0){
        printf("Error creating hyperslab\n");
        goto error;
    }
    status  = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, hyperslab_dataspace_id, dataspace_id, H5P_DEFAULT, *out_column_address);
    if(status < 0){
        printf("Error writing hyperslab\n");
        goto error;
    }
    status = H5Sselect_none(dataspace_id);
    if(status < 0){
        printf("Error undoing selection hyperslab\n");
        goto error;
    }

error:
    H5Sclose(hyperslab_dataspace_id);
    return status;
}
/*
    Read a 2D Dataset with fixed sizes from a HDF5 file into the Handler struct
    This function allocated memory in H5DatasetHandler->read_data_columns which needs to be freed!
    args:
        self: pointer returned by H5DatasetHandler_init
    return:
        SUCCESS if operation worked otherwise an enum with a nonzero value
*/
ErrorCode H5DatasetHandler_read_array_of_columns(struct H5DatasetHandler *self){
    
    ErrorCode return_val = SUCCESS;
    hid_t dataspace_id, dataset_id;
    herr_t status;
    hsize_t dims[2];
    hsize_t maxdims[2];
    int n_dims;

    dataset_id = H5Dopen(self->loc, self->name, H5P_DEFAULT);
    if (H5I_INVALID_HID == dataset_id){
        return_val = FAILURE;
        goto error;
    }
    dataspace_id = H5Dget_space(dataset_id);
    if(H5I_INVALID_HID == dataspace_id){
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }
   
    // Find rank and retrieve current and maximum dimension sizes.
    n_dims = H5Sget_simple_extent_dims(dataspace_id, dims, maxdims);
    if (n_dims < 0){
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }

    self->read_nrows = dims[0];
    self->read_ncols = dims[1];
    
    self->read_data_columns = malloc(sizeof(double*)*(self->read_ncols));
    
    if (NULL == self->read_data_columns){
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = OUTOFMEMORY;
        goto error;
    }

    for(hsize_t i = 0; i < self->read_ncols; i++){
        status = _read_column_from_dataset(dataset_id, dataspace_id, i, &(self->read_data_columns[i]));
        if (status < 0){
            break;
        }
    }
    if(status < 0){
        free(self->read_data_columns);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }

    H5Sclose(dataspace_id);
    H5Dclose(dataset_id);
    return return_val;

error:
    self->read_data_columns = NULL;
    self->read_nrows = -1;
    self->read_ncols = -1;
    return return_val;

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
    free(self->name);
    free(self);
    *self_addr = NULL;
}
