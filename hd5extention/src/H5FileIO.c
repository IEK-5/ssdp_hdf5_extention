#include <hdf5.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif
#include <unistd.h>
#include "H5Table.h"
#include "H5Enums.h"
#include "H5FileIO.h"
#include "H5Dataset.h"
#include "H5Datatypes.h"
/*  
    This is a refractor of example_4.c in which I did the following:

        This programm creates an hdf5 sile containing a group which contains two subgroups
        both subgroups each share a common dataset but also have their own datasets
        the shared dataset posses attributes

        --> I implemented the sharing as the dataset being in the first group and the second group having a link
    
    Now I want to do it again but this time I use structs to make the code more readable.
*/

/*
    What we need:
    
    FileHandler:
        members:
            - id of file
            - filename
            - mode (read-only, read-write)
            - datasets list pf DatasetHandlers
            (- groups list of GroupHandlers)
    
        functions:
            init - it handles creating the h5 file
            free - it handles freeing the h5 file and calling free on the handlers
            read_array - read dataset from HDF5 into a native C array
            write_array - write dataset from native C array into HDF5 file
            (add_GroupHandler - add group_handler)
            ()

    DatasetHandler:
        members:
            - id of dataset
            - id of dataspace
            - rank
            - dimensions
            - name of dataset

        functions:
            init - handles creation of the dataset and dataspace
            free - handles freeing of the dataset, dataspace and attributes
    
    ( Later also review this again
        GroupHandler:
        members:
            - id of group
            - groupname
            - handlers of the contents
        functions:
            init - handles creation of group
            free - handles freeing of the group and the contents
            add_DatasetHandler - adds a dataset_handler
    )

    (Attributes are added using the H5LTset_attribute_X functions)
    


*/












/*  
    Create a H5FileIOHandler which makes it easier to open and create HDF5 files.
    This functions allocates memory on the heap and opens resources. Free the struct using H5FileIOHandler_free!

    args:
        fn: name of the file to be opened or created
        mode: wether to create a new file, open a file for reading or open a file for editing
    return:
        pointer to initialized H5FileIOHandler struct

*/
struct H5FileIOHandler* H5FileIOHandler_init(const char* fn, IOMode mode){

    struct H5FileIOHandler* self;
    self = malloc(sizeof(*self));
    
    if (NULL == self){
        goto emalloc;
    }

    self->mode = mode;
    self->filename = strdup(fn);
    
    // todo change the mode to more ususually named words
    switch(mode){
        case X:
            self->file_id = H5Fcreate(fn, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
            break;
        case W:
            self->file_id = H5Fcreate(fn, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            break;
        case R:
            self->file_id = H5Fopen(fn, H5F_ACC_RDONLY, H5P_DEFAULT);
            break;
        case A:
            if (access(fn, F_OK) == 0) {
                // file exists
                self->file_id = H5Fopen(fn, H5F_ACC_RDWR, H5P_DEFAULT);
            } else {
                // file doesn't exist
                self->file_id = H5Fcreate(fn, H5F_ACC_EXCL, H5P_DEFAULT, H5P_DEFAULT);
            }
            
            break;
        // always add default for savety
        default:
            self->file_id = H5I_INVALID_HID;
            break;
    }
    
    if (self->file_id == H5I_INVALID_HID){
        goto efileio;
    }   

    return self;

efileio:
    free(self);

emalloc:
    return NULL;

 }

/*
    Free the memory allocated and the resources opended by H5FileIOHandler_init
    args:
        self: pointer to H5FileIOHandler struct created by H5FileIOHandler_init
*/
void H5FileIOHandler_free(struct H5FileIOHandler **self_addr){
    struct H5FileIOHandler *self = *self_addr;
    if (NULL != self){
        H5Fclose(self->file_id);
	free(self->filename);
        free(self);
        *self_addr = NULL;
    }
}

// ErrorCode H5FileIOHandler_discover_datasets(struct H5FileIOHandler *self){}

/*
    Read a 1D array of doubles which should be treated as a 2D Matrix from the dataset called dataset_name from the HDF5 file.
    This function allocates memory for it's output which must be freed by the user!
    args:
        self: pointer created by H5FileIOHandler_init
        dataset_name: name of the dataset in the HDF5 file
        out_data: pointer output array value
        out_nrows: number of rows of the Matrix
        out_ncols: number of columns of the Matrix
    return:
        SUCESS if it worked else a nonzero enum value
 */
ErrorCode H5FileIOHandler_read_array(struct H5FileIOHandler *self, const char *dataset_name, double **out_data, int* out_nrows, int* out_ncols){
    // failure if dataset with dataset_name does not exist
    struct H5DatasetHandler *dataset = H5DatasetHandler_init(dataset_name, self->file_id);
    ErrorCode err;
    err = H5DatasetHandler_read_array(dataset);
    if(SUCCESS == err){
        *out_data = dataset->read_data;
        *out_nrows = dataset->read_nrows;
        *out_ncols = dataset->read_ncols;
    }
    H5DatasetHandler_free(&dataset);
    return err;

}

/*
    Write a 1D array of doubles which should be treated as a 2D Matrix from to a hd5 file in as a data set named dataset_name.
    
    args:
        self: pointer created by H5FileIOHandler_init
        dataset_name: name of the dataset in the HDF5 file
        data: pointer pointer to data
        nrows: number of rows of the Matrix
        ncols: number of columns of the Matrix
        chunk_size: number of rows making up a chunk for IO purposes
        disk_datatype: H5 Datatype which will be saved in the file
    return:
        SUCESS if it worked else a nonzero enum value
 */
ErrorCode H5FileIOHandler_write_array(struct H5FileIOHandler *self, const char *dataset_name, double *data, int nrows, int ncols, hsize_t chunk_size, hid_t disk_datatype){
    struct H5DatasetHandler *dataset = H5DatasetHandler_init(dataset_name, self->file_id);
    ErrorCode err;
    err = H5DatasetHandler_write_array( dataset, data,  nrows,  ncols, disk_datatype, chunk_size);
    H5DatasetHandler_free(&dataset);
    // failure if drive is full
    return err;
}




// TODO check https://docs.hdfgroup.org/hdf5/v1_14/_h5_e__u_g.html#sec_error for error handling

/*
    Read a 1D array of doubles which should be treated as a table from the dataset called dataset_name from the HDF5 file.
    This function allocates memory for it's output which must be freed by the user!
    args:
        self: pointer created by H5FileIOHandler_init
        dataset_name: name of the dataset in the HDF5 file
        out_data: pointer to output array
        out_nrows: number of rows of the Matrix
        out_ncols: number of columns of the Matrix
        out_column_names: pointer to array of column names
    return:
        SUCESS if it worked else a nonzero enum value
    TODO
        - check if self in in read mode if not return a non success errorcode
 */
ErrorCode H5FileIOHandler_read_table(struct H5FileIOHandler *self, const char *dataset_name, double **out_data, int* out_nrows, int* out_ncols, char*** out_column_names){
    struct H5TableHandler *table = H5TableHandler_init(dataset_name, self->file_id);
    ErrorCode err = H5TableHandler_read_table(table);
    if(SUCCESS == err){
        *out_data = table->read_data;
        *out_nrows = table->read_nrows;
        *out_ncols = table->read_ncols;
        *out_column_names = table->read_columnnames;
    }
    free(table);
    return err;
}

/*
    Write a 1D array of doubles which should be treated as a Table to a hd5 file in as a dataset named dataset_name.
    
    args:
        self: pointer created by H5FileIOHandler_init
        dataset_name: name of the dataset in the HDF5 file
        data: pointer pointer to data
        nrows: number of rows of the Matrix
        ncols: number of columns of the Matrix
        column_names: array of column names
    return:
        SUCESS if it worked else a nonzero enum value
 */
ErrorCode H5FileIOHandler_write_table(struct H5FileIOHandler *self, const char *dataset_name, double *data, int nrows, int ncols, const char** columns_names, hsize_t chunk_size){
    struct H5TableHandler *table = H5TableHandler_init(dataset_name, self->file_id);
    ErrorCode err;
    err = H5TableHandler_write_table( table, data,  nrows,  ncols, columns_names, chunk_size);
    free(table);
    // failure if drive is full
    return err;
}


#define DEFAULTPOOLSIZE 5
#define POOLINCREMENT 5
/*
    Create an empty pool.
*/
struct  H5FileIOHandlerPool* H5FileIOHandlerPool_init(){

    struct H5FileIOHandlerPool *out;
    out = malloc(sizeof(*out));
    if (NULL == out) goto eout;

    out->poolsize = 5;
    out->handlers = calloc(sizeof(*out->handlers), DEFAULTPOOLSIZE);
    if (NULL == out->handlers) goto ehandlers;

    return out;
ehandlers:
    free(out);
eout:
    return NULL;
}

/*
    Clean up the pool and close any open handlers.

    args:
        self_addr: address of pointer to H5FileIOHandlerPool
    
    we use pointer to pointer to avoid double free
*/
void H5FileIOHandlerPool_free(struct  H5FileIOHandlerPool** self_addr){
    struct H5FileIOHandlerPool *self = *self_addr;
    H5FileIOHandlerPool_close_all_files(self);
    free(self->handlers);
    free(self);
    *self_addr = NULL;
}

/*
    get the index of a handler in the pool that deals with a file fn
*/
int _H5FileIOHandlerPool_get_handler_index(struct H5FileIOHandlerPool * pool, char *fn){
    for(int i = 0; i < pool->poolsize; i++){
        struct H5FileIOHandler* curr = pool->handlers[i];
        if(NULL != curr){
            if(strcmp(curr->filename, fn) == 0){
                return i;
            }
        }
    }
    return -1;
}

/*
    Return the index of the first position in handlers which is NULL.
    Return -1 if the pool has no NULL entries or if the pool has size 0.
*/
int _H5FileIOHandlerPool_get_next_free_index(struct H5FileIOHandlerPool * pool){
    for(int i = 0; i < pool ->poolsize; i++){
        if(NULL == pool->handlers[i]){
            return i;
        }
    }
    return -1;
}

/*
    Get a H5FileIOHander for a file called fn opened in IOMode mode.
    It the Handler does not exists create one.
    If it is already in the pool return the address from the pool.
    If it is already in the pool but opened under a different mode return
    or when there is not enough memory for a new FileHandler return NULL!
    args:
        self: pointer to pool
        fn: name of the file to be opened or created
        mode: wether to create a new file, open a file for reading or open a file for editing
    return:
        pointer to new Handler if the file is not already in the pool
        pointer to existing Handler if the file is already in the pool under the same IOMode
        NULL if the file is in the pool under a different IOMode or when creating the Handler fails
*/
struct H5FileIOHandler* H5FileIOHandlerPool_get_handler(struct H5FileIOHandlerPool *self, char *fn, IOMode mode){
    // check if handler already exists
    struct H5FileIOHandler *out;
    int index;
    
    index = _H5FileIOHandlerPool_get_handler_index(self, fn);
    if(index >= 0){
        out = self->handlers[index];
        if (out->mode == mode){
            return out;
        }else{
            return NULL;
        }   
    }

    // is the pool big enough?
    index = _H5FileIOHandlerPool_get_next_free_index(self);
    if(index < 0){
        // make the pool bigger
        struct H5FileIOHandler **tmp;
        tmp = realloc(self->handlers, (self->poolsize+POOLINCREMENT)*sizeof(*self->handlers));
        if(NULL == tmp){
            return NULL;
        }
        else{
            self->handlers = tmp;
        }
        for(int i = 0; i < POOLINCREMENT; i++){
            self->handlers[self->poolsize+i]=NULL;
        }
        // index at the beginning of the new part of the pool
        index = self->poolsize;
        // updated size
        self->poolsize+=5;
    }

    // create new handler
    out = H5FileIOHandler_init(fn, mode);
    self->handlers[index] = out;
    
    return out;
}

/*
    Close the handler which deals with the HDF5 file called fn
*/
// set the pointer to NULL if the handler is closed
void H5FileIOHandlerPool_close_file(struct H5FileIOHandlerPool *self, char* fn){
    int index = _H5FileIOHandlerPool_get_handler_index(self, fn);
    if(index < 0){
        return;
    }
    H5FileIOHandler_free(&self->handlers[index]); // this sets the entry in the pool to zero
}

/*
    Close all handlers in the pool
*/
// set the pointer to NULL if the handler is closed
void H5FileIOHandlerPool_close_all_files(struct H5FileIOHandlerPool *self){
    for(int i = 0; i < self->poolsize; i++){
        if(NULL != self->handlers[i]){
            H5FileIOHandler_free(&self->handlers[i]); // this sets the entry in the pool to zero
        }
    }
}
