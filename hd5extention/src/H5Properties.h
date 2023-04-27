#include <H5Ppublic.h>
#include <hdf5.h>

/*
    Create a property list for writing datasets
*/
hid_t H5P_create_dataset_proplist(int chunk_ndims, const hsize_t *chunk_dims);
