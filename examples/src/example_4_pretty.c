#include <hdf5.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

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
    OUTOFMEMORY = 1,
    FAILURE = -1,
    // add others with different negative values
} ErrorCode;

const char* ErrorCode_to_string(ErrorCode err){
    switch (err) {
        case SUCCESS:
            return "SUCCESS";
        case OUTOFMEMORY:
            return "OUTOFMEMORY";
        case FAILURE:
            return "GENERAL FAILURE";
        default:
            return "UNDEFINED ERROR VALUE";
    }
}

struct H5FileIOHandler{
    hid_t file_id;
    char* filename;
    IOMode mode;
};

/*
    This struct helps managing a HD5Dataset which holds a n \times m array
    The data is stored using 16-bit int to save diskspace => PRECISION MAY BE LOST

    create it using int
    write to file using write_array
    read from file using read_array
    when read read_X variables contain the data
*/
struct H5DatasetHandler{
    /* 
        TODO
        checkout how to do 16 bit float and use it instead of uint16_t
        float16 offers more flexibility e.g. also store coordinates in radians
        make datatype and digits optional -> Enum?
        main datatypes 16, 32 and 64 bit
    */
    time_t t;
    hsize_t read_nrows;
    hsize_t read_ncols;
    double *read_data;
    hid_t datatype;
    double digit_scale;
    char *name;
    hid_t loc;
};

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
    self->name = name;
    self->read_data = NULL;
    self->read_ncols = -1;
    self->read_nrows = -1;
    self->digit_scale = 10;
    self->datatype = H5T_NATIVE_UINT16; // values times 10 and round to int make sure it fits into range 2^16 - 1
    return self;
}

/*
    Write a continuous 1d array of doubles which is interpreted as a matrix into a HDF5 file using the H5DatasetHandler helper struct
    args:
        self: Handler object created by H5DatasetHandler_init
        data: pointer to array of doubles
        nrows: rows of matrix
        ncols: columns of matrix
    return:
        SUCESS if operation worked otherwise an enum with a nonzero value
*/ 
ErrorCode H5DatasetHandler_write_array(struct H5DatasetHandler *self, double* data, int nrows, int ncols){
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
    uint16_t *rounded_data = malloc(sizeof(uint16_t)*ncols*nrows);
    for(int i = 0; i<nrows; i++){
        for(int j = 0; j<ncols; j++){
            rounded_data[i*ncols+j] = (uint16_t) round((data[i*ncols+j]*self->digit_scale));
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

/*
    Read a 2D Dataset with fixed sizes from a HDF5 file into the Handler struct
    This function allocated memory in H5DatasetHandler->read_data which needs to be freed!
    args:
        self: pointer returned by H5DatasetHandler_init
    return:
        SUCESS if operation worked otherwise an enum with a nonzero value
*/
ErrorCode H5DatasetHandler_read_array(struct H5DatasetHandler *self){
    /*
        todos
    */

    ErrorCode return_val = SUCCESS;
    hid_t dataspace_id, dataset_id;
    herr_t status;
    hsize_t dims[2];
    hsize_t maxdims[2];
    uint16_t* buff;
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
    
    buff = malloc(sizeof(uint16_t)*dims[0]*dims[1]);

    if (NULL == buff){
        printf("%s\n","Error when calling malloc");
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = OUTOFMEMORY;
        goto error;
    }

    // todo create array out_data and fill from h5 file
    status = H5Dread(dataset_id, self->datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buff);

    if(status < 0){
        free(buff);
        printf("%s %d\n","Status code:", status);
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = FAILURE;
        goto error;
    }

    self->read_data = malloc(sizeof(double)*dims[0]*dims[1]);
    
    if (NULL == self->read_data){
        free(buff);
        printf("%s\n","Error when calling malloc");
        H5Sclose(dataspace_id);
        H5Dclose(dataset_id);
        return_val = OUTOFMEMORY;
        goto error;
    }

    for(int i = 0; i<self->read_nrows; i++){
        for (int j = 0; j<self->read_ncols; j++){
            self->read_data[i*(self->read_ncols) + j] = ((double) buff[i*(self->read_ncols) + j])/self->digit_scale;
        }
    }
    free(buff);
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
    we dont have this free because the memory is passed to the user and therfore we dont really need a free function anymore
    void H5DatasetHandler_free(struct H5DatasetHandler **self_addr){
        struct H5DatasetHandler *self = *self_addr;
        if(NULL != self->read_data){
            free(self->read_data);
        }
        free(self);
        *self_addr = NULL;
    }
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
struct H5FileIOHandler* H5FileIOHandler_init(char* fn, IOMode mode){

    struct H5FileIOHandler* self;
    self = malloc(sizeof(*self));
    
    if (NULL == self){
        goto emalloc;
    }

    self->mode = mode;
    self->filename = fn;
    
    herr_t status;
    // todo change the mode to more ususually named words
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

/*
    Free the memory allocated and the resources opended by H5FileIOHandler_init
    args:
        self: pointer to H5FileIOHandler struct created by H5FileIOHandler_init
*/
void H5FileIOHandler_free(struct H5FileIOHandler *self){
   
    herr_t status = H5Fclose(self->file_id);
    free(self);
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
        out_columns: number of columns of the Matrix
    return:
        SUCESS if it worked else a nonzero enum value
 */
ErrorCode H5FileIOHander_read_array(struct H5FileIOHandler *self, const char *dataset_name, double **out_data, int* out_nrows, int* out_ncols){
    // failure if dataset with dataset_name does not exist
    struct H5DatasetHandler *dataset = H5DatasetHandler_init(dataset_name, self->file_id);
    ErrorCode err;
    err = H5DatasetHandler_read_array(dataset);
    if(SUCCESS == err){
        *out_data = dataset->read_data;
        *out_nrows = dataset->read_nrows;
        *out_ncols = dataset->read_ncols;
    }
    free(dataset);
    return err;

}

ErrorCode H5FileIOHandler_write_array(struct H5FileIOHandler *self, const char *dataset_name, double *data, int nrows, int ncols){
    struct H5DatasetHandler *dataset = H5DatasetHandler_init(dataset_name, self->file_id);
    ErrorCode err;
    err = H5DatasetHandler_write_array( dataset, data,  nrows,  ncols);
    free(dataset);
    // failure if drive is full
    return err;
}



// TODO check https://docs.hdfgroup.org/hdf5/v1_14/_h5_e__u_g.html#sec_error for error handling

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