#include <H5Ipublic.h>
#include <H5public.h>
#include <hdf5.h>
#include <stdlib.h>

/*
	This programm creates an hdf5 sile containing a dataset with a dataspace space of rank 4 times 6 containing zeros
*/ 

int main(){
    // exit code when closing stuff
    herr_t status;
    // dimension of array
    hsize_t dims[2];
    // id of dataset
    hid_t file_id;
    // id of dataset
    hid_t dataset_id;
    // id of dataspace
    hid_t dataspace_id;
    // Create the dataspace for the dataset.
    dims[0] = 4;
    dims[1] = 6;
    dataspace_id = H5Screate_simple(2, dims, NULL);

    // create a HD5File
    file_id = H5Fcreate ("file2.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

    // Create the dataset.
    dataset_id = H5Dcreate (file_id, "/dset", H5T_STD_I32BE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
    // Close the dataset and dataspace
    status = H5Dclose(dataset_id);
    status = H5Sclose(dataspace_id);
    exit(0);
}
