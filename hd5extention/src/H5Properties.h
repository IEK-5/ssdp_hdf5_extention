#include <H5Ppublic.h>
#include <hdf5.h>

/*
    Create a property list for writing datasets
*/
hid_t H5P_create_dataset_proplist(int chunk_ndims, const hsize_t *chunk_dims);

hid_t H5P_create_access_dataset_proplist(const hsize_t *chunk_dims);

hid_t H5P_create_16_MB_Chunk_Cache_access_dataset_proplist();