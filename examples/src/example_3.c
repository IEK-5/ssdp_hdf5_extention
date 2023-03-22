#include <H5Dpublic.h>
#include <H5Fpublic.h>
#include <H5Ipublic.h>
#include <H5Ppublic.h>
#include <H5public.h>
#include <H5version.h>
#include <hdf5.h>
#include <stdlib.h>
#include <stdio.h>

/*
	This programm creates an hdf5 sile containing a dataset with a dataspace space of rank 4 times 6 containing the numbers 1 to 23
    then it opens this file and reads the values into a c array
*/

void print_array(int *array, int length)
{
    for (int i = 0; i < length; i++) {  printf("%i ",array[i]);}
    printf("\n");
}

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

    // define the filename
    const char file_name[] = "file3.h5";
    // a priori knowledge of the dataset name
    const char dset_name[] = "/dset";
    // a priori knowledge of the data type
    hid_t mem_type_id = H5T_NATIVE_INT;
    // define dataset_dataset_data_array
    int dset_data_write[dims[0]*dims[1]];
    int dset_data_read[dims[0]*dims[1]];

    // create a HD5File
    file_id = H5Fcreate (file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

         dataspace_id = H5Screate_simple(2, dims, NULL);
        // Create the dataset.
            dataset_id = H5Dcreate (file_id, dset_name, H5T_STD_I32BE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                int k = 0;
                for (int i = 0; i < dims[1]; i++){
                    for (int j = 0; j < dims[0]; j++){
                        dset_data_write[i*dims[0]+j] = k;
                        k+=1;
                    }
                }
                H5Dwrite (dataset_id, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data_write);

                // Close the dataset and dataspace
            status = H5Dclose(dataset_id);
        status = H5Sclose(dataspace_id);
    status = H5Fclose(file_id);

    file_id = H5Fopen(file_name, H5F_ACC_RDONLY, H5P_DEFAULT);
        if (file_id == H5I_INVALID_HID){
            exit(1);
        }

        dataset_id = H5Dopen(file_id, dset_name, H5P_DEFAULT);
            if (dataset_id == H5I_INVALID_HID){
                exit(1);
            }
            
            // TODO figure out what H5Dread does and how we get the c array out of the dataspace
            // read all dataset elements
            if ((status = H5Dread(dataset_id, mem_type_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, dset_data_read)) < 0){
                exit(status);
            }

            int *foo = (int*) malloc(sizeof(int)*10);
            // do something w/ the dataset elements
            printf("written data\n");
            print_array(dset_data_write, dims[0]*dims[1]);
            printf("read data\n");
            print_array(dset_data_read, dims[0]*dims[1]);
        H5Dclose(dataset_id);
    H5Fclose(file_id);
    
    exit(0);
}
