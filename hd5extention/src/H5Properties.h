#include <H5Ipublic.h>
#include <H5Ppublic.h>
#include <hdf5.h>

/*
    Create a property list for writing datasets
    args:
        chunk_ndims: number of dimens the chunk this must be the length of chunk_dims
        chunk_dims: the size of the chunk in each dim
    
    e.g. if the dataset is a nrows x ncols matix and you wish to chunk together 2 columns and 5 rows
    you choose chunk_ndims = 2 and chunk_dims = {}
*/
hid_t H5P_create_dataset_proplist(int chunk_ndims, const hsize_t *chunk_dims);

hid_t H5P_create_16_MB_Chunk_Cache_access_dataset_proplist();
