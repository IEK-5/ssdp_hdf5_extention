#include <hdf5.h>
#include <hdf5_hl.h>
#include <stdio.h>
#include <stdlib.h>
#include "arraylist.h"

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

typedef enum {
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
    struct DatasetHandler* datasets;
};

struct H5FileIOHandler* H5FileIOHandler_init(char* fn, IOMode mode, char ** datasets){

 };

ErrorCode H5FileIOHandler_free(struct H5FileIOHandler *self){

}

ErrorCode H5FileIOHander_read_array(struct H5FileIOHandler *self, char *dataset_name, double **out_data, int *out_dims, int *out_rank){

}

ErrorCode H5FileIOHandler_write_array(struct H5FileIOHandler *self, char *dataset_name, double **in_data, int *in_dims, int rank){

}

struct DatasetHandler{

};

void print_array(int *array, int length)
{
    for (int i = 0; i < length; i++) {  printf("%i ",array[i]);}
    printf("\n");
}

int main(){
    int *arr = (int *)malloc(sizeof(int)*4);
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
    printf("%d\n", *result);


    free(arr);
}