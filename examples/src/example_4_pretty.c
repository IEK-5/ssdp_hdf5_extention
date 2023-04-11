#include <hdf5.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

# define ADDR_OF(X) ((int[]){X})

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

void print_array(int *array, int length){
    for (int i = 0; i < length; i++){
        printf("%i ",array[i]);
    }
    printf("\n");
}



typedef enum {
    CREATE,
    WRITE,
    READ,
} IOMode;

// Maybe use H5E_values ?
typedef enum {
    SUCCESS = 0,
    FAILURE = -1,
    // add others with different negative values
} ErrorCode;

struct H5FileIOHandler{
    hid_t file_id;
    char* filename;
    IOMode mode;
};

struct H5DatasetHandler{
    /*
        This struct helps managing a HD5Dataset which holds a n \times m array
        The data is stored using 16-bit int to save diskspace => PRECISION MAY BE LOST

        create it using int
        write to file using write_array
        read from file using read_array
        when read read_X variables contain the data
        
    */
    hsize_t read_nrows;
    hsize_t read_ncols;
    double **read_data;
    hid_t datatype;
    int digit_scale;
    char *name;
    hid_t loc;
};

struct H5DatasetHandler* H5DatasetHandler_init(char *name, hid_t loc){
    /*
        args:
            name: name of dataset should be like a unix path e.g. /foo/bar/datasetname
            loc: hid_t of the H5D file in which the dataset lives
    */
    struct H5DatasetHandler* self = malloc(sizeof(struct H5DatasetHandler));
    if (NULL == self){
        return NULL;
    }
    self->loc = loc;
    self->name = name;
    self->read_data = NULL;
    self->read_ncols = -1;
    self->read_nrows = -1;
    self->digit_scale = 10;
    self->datatype = H5T_NATIVE_UINT16; // values times 10 and round to int make sure it fits into range 2^16 - 1
    return self;
}

ErrorCode H5DatasetHandler_write_array(struct H5DatasetHandler *self, double** data, int nrows, int ncols){
    /*
        write a 2d c-array of doubles into a hdf5file
    */

    // write the 2d c-array data  into the HDF5 File using the paramaters in self

    /*
        TODOs:
            what do we do if a dataset already exists?
            check if the values are in the correct range to fit into a 16 bit int
    */
    hsize_t dims[2];
    herr_t status;
    dims[0]=nrows;
    dims[1]=ncols;
    hid_t dataspace_id, dataset_id;
    dataspace_id = H5Screate_simple(2,dims,NULL);
    if(H5I_INVALID_HID == dataspace_id){
        return FAILURE;
    }
    dataset_id = H5Dcreate(self->loc, self->name, self->datatype, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (H5I_INVALID_HID == dataset_id){
        H5Sclose(dataspace_id);
        return FAILURE;
    }
    uint16_t **rounded_data = malloc(sizeof(uint16_t)*ncols*nrows);
    for(int i = 0; i<ncols; i++){
        for(int j = 0; j<nrows; j++){
            rounded_data[i][j] = (uint16_t) round((data[i][j]*self->digit_scale));
        }
    }
    status  = H5Dwrite(dataset_id, self->datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, rounded_data);
    H5Dclose(dataset_id);
    H5Sclose(dataspace_id); // todo is it ok to close it here already?
    if (0 > status){
        printf("%s %d\n","Status code:", status);
        return FAILURE;
    }
    else{
        return SUCCESS;
    }
}

ErrorCode H5DatasetHandler_read_array(struct H5DatasetHandler *self){
    // read a 2d dataset from the file using the specifications in self

    /*
        todos
    */
    hid_t dataspace_id, dataset_id;
    herr_t status;
    hsize_t dims[2];
    hsize_t maxdims[2];
    uint16_t* buff;
    int n_dims;

    dataset_id = H5Dopen(self->loc, self->name, H5P_DEFAULT);
    if (H5I_INVALID_HID == dataset_id){
        goto error;
    }
    dataspace_id = H5Dget_space(dataspace_id);
    if(H5I_INVALID_HID == dataspace_id){
        H5Dclose(dataset_id);
        goto error;
    }
   
    // Find rank and retrieve current and maximum dimension sizes.
    n_dims = H5Sget_simple_extent_dims(dataspace_id, dims, maxdims);
    if (n_dims < 0){
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        goto error;
    }

    self->read_nrows = dims[0];
    self->read_ncols = dims[1];
    
    buff = malloc(sizeof(uint16_t)*dims[0]*dims[1]);

    if (NULL == buff){
        printf("%s\n","Error when calling malloc");
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        goto error;
    }

    // todo create array out_data and fill from h5 file
    status = H5Dread(dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, buff);

    if(status < 0){
        free(buff);
        printf("%s %d\n","Status code:", status);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        goto error;
    }

    self->read_data = malloc(sizeof(double)*dims[0]*dims[1]);
    
    if (NULL == self->read_data){
        free(buff);
        printf("%s\n","Error when calling malloc");
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        goto error;
    }

    for(int i = 0; i<self->read_ncols; i++){
        for (int j = 0; j<self->read_nrows; j++){
            self->read_data[i][j] = buff[i*(self->read_ncols) + j]/10;
        }
    }
    free(buff);
    H5Sclose(dataspace_id);
    H5Dclose(dataset_id);
    return SUCCESS;

error:
    self->read_data = NULL;
    self->read_nrows = -1;
    self->read_ncols = -1;
    return FAILURE;

}

void H5DatasetHandler_free(struct H5DatasetHandler **self_addr){
    struct H5DatasetHandler *self = *self_addr;
    if(NULL != self->read_data){
        free(self->read_data);
    }
    free(self);
    *self_addr = NULL;
}


struct H5FileIOHandler* H5FileIOHandler_init(char* fn, IOMode mode){
    struct H5FileIOHandler* self;
    self = malloc(sizeof(*self));
    
    if (NULL == self){
        goto emalloc;
    }

    self->mode = mode;
    self->filename = fn;
    
    herr_t status;
    switch(mode){
        case CREATE:
            self->file_id = H5Fcreate(fn, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            break;
        case READ:
            self->file_id = H5Fopen(fn, H5F_ACC_RDONLY, H5P_DEFAULT);
            break;
        case WRITE:
            self->file_id = H5Fopen(fn, H5F_ACC_RDWR, H5P_DEFAULT);
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

 };

void H5FileIOHandler_free(struct H5FileIOHandler *self){
    herr_t status = H5Fclose(self->file_id);
    free(self);
}

// ErrorCode H5FileIOHandler_discover_datasets(struct H5FileIOHandler *self){}

ErrorCode H5FileIOHander_read_array(struct H5FileIOHandler *self, char *dataset_name, double **out_data, int nrows, int ncols, int out_rank){
    // failure if dataset with dataset_name does not exist
    struct H5DatasetHandler *dataset = H5DatasetHandler_init(dataset_name, self->file_id);
    ErrorCode err;
    err = H5DatasetHandler_read_array(dataset);
    if(FAILURE == err){

    }
    

}

ErrorCode H5FileIOHandler_write_array(struct H5FileIOHandler *self, char *dataset_name, double **in_data, int *in_dims, int rank){
    // failure if drive is full
    return FAILURE;
}



// TODO check https://docs.hdfgroup.org/hdf5/v1_14/_h5_e__u_g.html#sec_error for error handling

int main(){
    /*
    int *arr = (int *) malloc(sizeof(int)*4);
    for(int i = 0; i<4; i++){
        arr[i]=i;
    }
    print_array(arr, 4);
    arr = (int *) realloc(arr, sizeof(&arr)*5);
    arr[4] = 42;
    print_array(arr, 5);
    arraylist* mylist = arraylist_create();
    arraylist_add(mylist, ADDR_OF(1));
    arraylist_add(mylist, ADDR_OF(2));
    int* result = (int *) arraylist_pop(mylist);
    printf("%d\n",(int) *result);
    free(arr);
    */
    return 0;
}