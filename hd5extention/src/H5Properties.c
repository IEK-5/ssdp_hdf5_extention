#include <H5Ipublic.h>
#include <H5Ppublic.h>
#include <hdf5.h>
#include <stdlib.h>
#include "H5Properties.h"

hid_t H5P_create_dataset_proplist(int chunk_ndims, const hsize_t *chunk_dims){
    herr_t status;
    hid_t dset_create_props;
    dset_create_props = H5Pcreate (H5P_DATASET_CREATE);

    status = H5Pset_chunk (dset_create_props, 2, chunk_dims);
    if(status < 0) {
        goto error;
    }
    status = H5Pset_nbit(dset_create_props);
    if(status < 0) {
        goto error;
    }
    status = H5Pset_deflate(dset_create_props, 9);
    if(status < 0) {
        goto error;
    }
    
    return dset_create_props;
error:
    H5Pclose(dset_create_props);
    return H5I_INVALID_HID;
}

hid_t H5P_create_access_dataset_proplist(const hsize_t *chunk_dims){
    /*
    TODO

    determine chunk size based on chunk dims
    for that we need to find a matching prime and calculate rdcc_nslots and rdcc_nbytes properly
    according to https://docs.hdfgroup.org/hdf5/v1_14/group___d_a_p_l.html#ga104d00442c31714ee073dee518f661f1
    */

    
    /*
    herr_t status;
    hid_t dset_access_props;
    dset_access_props = H5Pcreate (H5P_DATASET_ACCESS);
    status = H5Pset_chunk_cache(dset_access_props, 12421, 16*1024*1024, H5D_CHUNK_CACHE_W0_DEFAULT);
     if(status < 0) {
        goto error;
    }
    
    return dset_access_props;
error:

    H5Pclose(dset_access_props);
    */
    return H5I_INVALID_HID;
}

hid_t H5P_create_16_MB_Chunk_Cache_access_dataset_proplist(){
    /*
    Example for creating a dataset acess plist with a 16 MB chunk cache
    taken from
    https://docs.hdfgroup.org/hdf5/v1_14/group___d_a_p_l.html#ga104d00442c31714ee073dee518f661f1
    */
    herr_t status;
    hid_t dset_access_props;
    dset_access_props = H5Pcreate (H5P_DATASET_ACCESS);
    status = H5Pset_chunk_cache(dset_access_props, 12421, 16*1024*1024, H5D_CHUNK_CACHE_W0_DEFAULT);
     if(status < 0) {
        goto error;
    }
    
    return dset_access_props;
error:
    H5Pclose(dset_access_props);
    return H5I_INVALID_HID;
}