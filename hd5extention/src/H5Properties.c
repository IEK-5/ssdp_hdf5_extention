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
    return dset_create_props;
error:
    H5Pclose(dset_create_props);
    return H5I_INVALID_HID;
}