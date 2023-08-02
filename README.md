# SSDP HDF5 Extention

This is a library developed to be used in conjunction with [SSDP](https://github.com/IEK-5/ssdp) to allow input and output via the [HDF5 file format](https://www.hdfgroup.org/solutions/hdf5/). It can also be used independently from SSDP hence it is its own project. The main aim of this library is to provide an easier access to some IO features of HDF5.

## Project Status

This library is still being actively developed. 

## Table of Contents

1. [Installation](#Installation)
2. [Usage](#Usage)
3. [Features](#Features)
4. [Code Examples](#Code-Examples)
5. [Development](#Development)
6. [License](#License)
7. [Troubleshooting and FAQs](#FAQs)
8. [Contact Information](#Contact-Information)

## Installation

For installing and developing this project is configured to use [Devenv](https://devenv.sh/getting-started/#initial-set-up) to manage the dependencies combined with [Direnv](https://direnv.net/docs/hook.html) to automatically start up the development environment. Both tools are configured using the files: `devenenv.nix`, `devenenv.yaml`, `devenenv.lock` and `.envrc`.

The file `devenv.nix` contains the dependencies. The file `devenv.yaml`configures inputs and imports.
The files `devenv.lock` is created from the nix and yaml files. It pins the inputs making the environment reproducible.
The `.envrc` file tells direnv it should actviate the env.

For installation without [Devenv](https://devenv.sh/getting-started/#initial-set-up) the following dependencies are needed:

1. gcc or clang C or another C compiler compatible with HDF5's `h5cc` wrapper script.
2. [GNU make](https://www.gnu.org/software/make/)
3. [HDF5-1.14.0 or higher](https://www.hdfgroup.org/downloads/hdf5)
4. [Zlib](https://www.zlib.net/)
5. [Criterion Test Framework](https://criterion.readthedocs.io/en/master/intro.html)

To build the software navigate to the folder `hd5extention` and call:

```
make
```

or

```
make all
```

This will create all objects files in a folder `obj` and a statically linked library file in a folder `lib`.


To compile and run the tests call:

```
make tests
```

To just compile tests call:

```
make testscompile
```

*NOTE*: This software has only been compiled and tested under: Archlinux, Ubuntu 20.04 LTS and Fedora.

To remove all files created by `make` call:

```
make clean
```
## Usage

This library uses struct and helper functions for said struct to make the usage of HDF5 easier.

Most use cases can be covered by the structs and functions in `H5FileIO.h` and the enums in `H5Enums.h`.


`H5FileIO.h` has a central struct called `H5FileIOHandler`. It provides functions to read datasets from HDF5 Files (and tables WIP) into
C arrays and write C arrays into datasets. There exists also a struct called `H5FileIOHandlerPool` which eases the creation and management of multiple `H5FileIOHandler` structs. Each struct in this library has an `<structname>_init` and `<structname>_free` function used for creating and freeing instances of the struct respectively.

`H5Enums.h` provides two important enums: `IOMode` and `ErrorCode`. `IOMode` is used to determin how a HDF5 file should be opened. `ErrorCode` is used as a return value for many functions in this library.

Each function has a doc string. In section [Code Examples](#Code-Examples) you also find example which help understand how to use this library.


## Features

- Creation and reading of HDF5 Files
- Creation and reading of datasets. See `H5FileIOHander_[write|read]_array_of_columns`.
    - Datasets are created using chunked IO, N-Bit Filter and gzip compression
- Creation and reading of tables. See `H5FileIOHander_[write|read]_table` **WIP**.
- Management of multiple HDF5 Files using `H5FileIOHanderPool`.
- Creation of n-bit-floating point datatypes using `H5T_define_nbit_float` in `H5Datatypes.h`
- Configuration of Chunk-Cache (TODO see `H5P_create_16_MB_Chunk_Cache_access_dataset_proplist` in `H5Properties.h`)


## Code Examples

### Writing data into an HDF5 file
This program creates a file called `myfile.h5` and writes a $3 \times 3$ array into a dataset called "dataset1".

```
#include <stdio.h>
#include <stdlib.h>
#include <hdf5.h>
#include "H5FileIO.h"
#include "H5Enums.h"

# define COLS 3
# define ROWS 3

int main(void){
    ErrorCode err;
    int retval = 0;
    double **myArray;
    
    // create a handler
    // IO_W means write and if exists overwrite
    struct H5FileIOHandler *handler = H5FileIOHandler_init("myfile.h5", IO_W);

    // if init fails it returns NULL
    if (NULL == handler){
        printf("Couldn't create handler\n");
        return -1;
    }

    /*
    0 3 6
    1 4 7
    2 5 8
    */
    // arrays must be dynamically allocated!
    // outer array contains pointers to columns
    // columns are 1d arrays
    myArray = malloc(sizeof(*myArray)*COLS);
    for(int i = 0; i < COLS; i++){
        myArray[i]=malloc(sizeof(**myArray)*ROWS);
        for(int j = 0; j < ROWS; j++){
            myArray[i][j]=i*ROWS+j;
        }
    }

    // write array
    err = H5FileIOHandler_write_array_of_columns(handler, "dataset1",
            myArray, ROWS, COLS, 1, H5T_NATIVE_DOUBLE);
    
    // test for SUCCESS
    if (SUCCESS != err){
        printf("Error while writing array: ErrorCode %s\n", ErrorCode_to_string(err));
        retval = -1;
    }
    // release handler
    H5FileIOHandler_free(&handler);

    // cleanup array
    for(int i = 0; i < COLS; i++){
        free(myArray[i]);
    }
    free(myArray);
    return retval;
    
}
```
### Reading data from an HDF5 file

This program assume there exists a file `myfile.h5` created by the previous program. The file contains a dataset called `dataset1`. It is being read into a C array.

```
#include <stdio.h>
#include <stdlib.h>
#include <hdf5.h>
#include "H5FileIO.h"
#include "H5Enums.h"

#define MAXNCOLS 100

int main(void){
    ErrorCode err;
    int retval = 0;
    double **myArray = NULL;
    int ncols = -1, nrows = -1;
    // create a handler
    // IO_R means read only
    struct H5FileIOHandler *handler = H5FileIOHandler_init("myfile.h5", IO_R);

    // if init fails it returns NULL
    if (NULL == handler){
        printf("Couldn't create handler\n");
        return -1;
    }


    // read array
    // pass the adress of myArray!
    // myArray now get's filled and must be freed!
    err = H5FileIOHandler_read_array_of_columns(handler, "dataset1",
            &myArray, &nrows, &ncols, MAXNCOLS);
    
    // test for SUCCESS
    if (SUCCESS != err){
        printf("Error while reading array: ErrorCode %s\n", ErrorCode_to_string(err));
        retval = -1;
    }
    // release handler
    H5FileIOHandler_free(&handler);

    if (NULL != myArray){
        // print array
        for(int i = 0; i < ncols; i++){
            for(int j = 0; j < nrows; j++){
                printf("%lf\t",myArray[i][j]);
            }
            printf("\n");
        }
        // cleanup array
        for(int i = 0; i < ncols; i++){
            free(myArray[i]);
        }
        free(myArray);
        }
    return retval;
}
```

### Handling multiple HDF5 Files

Suppose you have a program in which you need to work on multiple files.
You need to write and read multiple datasets from multiple files.
To make your life easier use `H5FileIOHandlerPool` to not worry about
opening and closing each handler manually.

```
#include <H5Tpublic.h>
#include <stdio.h>
#include <stdlib.h>
#include <hdf5.h>
#include "H5FileIO.h"
#include "H5Enums.h"

# define MAXNCOLS 100

struct _Helper{
    char *filename;
    char *dataset_in;
    char *dataset_out;
    int chunksize;
};

struct _Helper get_userinput(void);

/*
Perform some important calculation inplace.
*/
void some_important_calculation(double ***array_of_columns, int nrows, int ncols);

void free_data(double **data, int ncols){
    for(int i = 0; i < ncols; i++){
        free(data[i]);
    }
    free(data);
}

/*
    This function parses the user input
    to:
    1. read a dataset
    2. perform some important calculation
    3. save the result of the calculation in another dataset

*/
void process_file(struct H5FileIOHandlerPool *pool){
    ErrorCode err;
    struct _Helper h;
    struct H5FileIOHandler *handler;
     double **data = NULL;
    int nrows = -1, ncols = -1;


    h = get_userinput();
    // IO_A means read write
    handler = H5FileIOHandlerPool_get_handler(pool, h.filename, IO_A);
    if (NULL != handler){
        err = H5FileIOHandler_read_array_of_columns(handler, h.dataset_in, &data, &nrows, &ncols, MAXNCOLS);
        if (SUCCESS == err){
            some_important_calculation(&data, nrows, ncols);
            H5FileIOHandler_write_array_of_columns(handler, h.dataset_out, data, nrows, ncols, h.chunksize, H5T_NATIVE_DOUBLE);
        }
        free_data(data, ncols);
    }
}

int main(void){
    
    struct H5FileIOHandlerPool *pool = H5FileIOHandlerPool_init();
    if(NULL == pool){
        printf("Coulnd't initiate pool");
        return -1;
    }
    
    char input;

    while (1) {
        printf("Type C to continue, F to flush or Q to quit: ");
        scanf(" %c", &input);

        if (input == 'C' || input == 'c') {
            process_file(pool);
        } else if (input == 'F' || input == 'f') {
            H5FileIOHandlerPool_close_all_files(pool);
        } else if (input == 'Q' || input == 'q') {
            printf("Terminating...\n");
            break;
        } else {
            printf("Invalid input. Please try again.\n");
        }
    }

    

    H5FileIOHandlerPool_free(&pool);

}
```

## Development

The following conventions and techniques are used:

- Functions and structs which wrap HDF5 start with H5
- Struct have an `_init` and `_free` function
- `_free` function accepts **adresses of pointers** to set the underlying pointer to `NULL`
- `ErrorCode` enum is used for return values
- features should be tested using criterion test framework in a file in the `tests` folder starting with the following naming convention `test_<name>.c`
- If a test creates files they should be located in `tests/testfiles` to be cleaned up by make.
- Most user API functions and structs should be in `H5FileIO.h`, library internal functions and structs such as `H5DatasetHandler` should not be there.
- If the creation/configuration of a HDF5 file requires code which creates ressourced identitied by `hid_t` it's usually a good idea to seperate it into different functions and files, e.g see `H5Proteries.h`, `H5Dataset.h`, `H5Datatype.h`.
- Use goto and tags to avoid memory leaks e.g. see `H5DatasetHandler_write_array_of_columns`.

### Roadmap
In no particular order.

- Finish implementation of H5Table, to be compatible with array of columns
- Implement automatic determination of optimal chunk-cache based on chunk size

## License

## Troubleshooting and FAQs

## Contact Information
[Michael Gordon, Forschungszentrum Jülich GmbH, IEK-5](m.gordon@fz-juelich.de)
[Dr. Evgenii Sovetkin, Forschungszentrum Jülich GmbH, IEK-5](e.sovetkin@fz-juelich.de)

