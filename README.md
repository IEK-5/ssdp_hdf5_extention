# SSDP HDF5 Extention

## Concept

This repo is a playground for implementing HDF5 in C for a later feature to be included in SSDP.

## Setup

Here I use [Devenv](https://devenv.sh/getting-started/#initial-set-up) to manage the dependencies.
I also use [Direnv](https://direnv.net/docs/hook.html) to automatically start up the env.
The file `devenv.nix` contains the dependencies. The file `devenv.yaml`configures inputs and imports that means it tells us from where we get our imports.
The files `devenv.lock` is created from the nix and yaml files. It pins the inputs making the environment reproducible.
The `.envrc` file tells direnv it should actviate the env.

## About HDF5

- it's a flexible data format capable of storing data and metadata of various kinds in a space efficient way
- it can be thought of a database 

### Data Model

- a HDF5 file if a container / database of heterogenous data objects (datasets)
- examples of datasets are: images, tables, graphs, binary documents such as PDF and Excel or other files
- the main data objects are groups and datasets
- other data objects are datatypes, dataspaces, properties and attributes

### Groups

- organize data objects
- can be thought of as directories from linux
- each hdf5 file has a root group `/`
- each file may contain groups within groups (all groups live at least within root)
- data objects may live in multiple groups (hard links in multiple dirs to the same file/inode)

### Datasets

- organize and contain "raw" data as well as metadata
- bascially an n-dimensional array of fixed or with unlimited (extentible) size with metadata
- meta data is a dataspace which tells us the dimensions, the datatype, attributes and properties

### Datastpace

- describes the layout of the dataset's data elements (the shape of the array)#
- two roles
    - logical layout: rank and dimensions
    - describes the data buffers for I/O e.g. subset/index of dataset ( e.g. 2x2 subset of a 10x10 matrix)

### Properties

- characteristics or features of HDF5 object
- `interfaces` in oo language
- HDF5 Property List API
- examples
    1. contiguous: elements are adjacent to each other in memory
    2. chunked: only data within subsets is adjacent in memory -> faster acess
    3. chunked & compressed: like chunked but less storage on disk

### Attributes

- key value pairs attached to an object -> not independant
- used for metadata
- also habe datatype and dataspace but do not support partial I/O and can not be compressed or extended

### Datatypes

- describe datatype of data in a dataset

#### pre-defined

- hdf5 works with these "under the hood"
- 2 types
    - standard, which are all the same independent of platform, nomenclature: `H5T_ARCH_BASE` e.g. `H5T_IEEE_F32BE` floating point big endian
    - native, not the same on different platforms e.g. `H5T_NATIVE_INT` integer in C

#### derived

- created or derived (idk whats the difference) from pre-defined
- examples are strings (multiples chars) or combound e.g. int16+char+int32+2x3x2 array of float32 (you can combine whatever however you want I think)
- compound datatypes can consist of ther compound types
- compound types can be stored in arrays (dataspaces) just like pre-defined types

## SSDP IO

- filename, double **array, columns, rows
- ssdp does know only 1d arrays
- columns of csvs are up to the user to be assigned to names and internally treated as 1d arrays
- ssdp uses double for everything

### arrayparser.c

- make WriteArraysToHD5 function similar to WriteArraysToFile with a docstring which generates stuff for ssdp language

## Goals

- first version is just hdf version of output to ascii
- second version is to give columns names

- later versions in order of priority
    - configurable datatypes
    - hdf5 output simulation script and input files and implement routine to to resume/run simulation from ssdp output hdf5 files

## How to develop and debug

- use valgrind to find leaks
- c debugger gdb
- speichern anlegen mit malloc, calloc und realloc
- compile with `-g` for debugging with valgrind
- pointers mit null initialisieren und bei free wieder auf null setzen und auf null abfragen
- macros mit "hacks" wie malloc return prüfen oder free und auf null setzen
- learn CI für C language
- maybe make debug makefile
- use valgrind especially when there is segfault

### Tables

- research what is HDF5 Table
- find out which kind of objects are columns

struct hdfio {
    hid_t fd;
    char mode;
};

struct *hdfio hdfio_init(char* fn, char mode, char **cols)
{
    struct hdfio *self;
    self = malloc(sizeo(*self));
    if (NULL == self)
        goto eself;
    self->mode = mode;
    // copy self-> cols;
    if ('w' == mode) {
        self->fd = 
        if (self->fd failed)
            goto efd;
    }

    return self;
efd:
    free(self);
eself:
    return NULL;
}

void hdfio_free(struct hdfio *self)
{
    // hdf_close(fd);
    // free cols
    free(self);
    self = NULL;
}

int hdfio_readarray(struct hdfio *self, int Ncols, double **df, int *nrow)
{

}

void test()
{
    struct hdfio* dh = hdfio_init("test.hdf5", 'w');
    if (NULL == dh)
             goto edf;

edf:
    hdfio_free(dh);
}