#include <H5Apublic.h>
#include <H5Dpublic.h>
#include <H5Fpublic.h>
#include <H5Gpublic.h>
#include <H5Ipublic.h>
#include <H5Lpublic.h>
#include <H5Ppublic.h>
#include <H5Spublic.h>
#include <H5Tpublic.h>
#include <H5public.h>
#include <H5version.h>
#include <hdf5.h>
#include <stdlib.h>
#include <stdio.h>

/*
	This programm creates an hdf5 sile containing a group which contains two subgroups
    both subgroups each share a common dataset but also have their own datasets
    the shared dataset posses attributes

    --> I implemented the sharing as the dataset being in the first group and the second group having a link
*/

void print_array(int *array, int length)
{
    for (int i = 0; i < length; i++) {  printf("%i ",array[i]);}
    printf("\n");
}

void check_status(herr_t status){
    if(status != 0){
        fprintf(stderr, "Status code: %d\n", status);
        exit(status);
    }
}

void fill_array(int* array, hsize_t* dims, int rank, int start_val){
    int k = start_val;
    int n = 1;
    for (int i = 0; i<rank; i++){
        n*=dims[i];
    }
    for(int i = 0; i<n; i++){
        array[i]=k;
        k++;
    }
}

int main(){
    // exit code when closing stuff
    herr_t status;
    // data array
    hsize_t dims[2];
    int rank = 2;
    dims[0] = 2;
    dims[1] = 2;
    int shared_data[dims[0]][dims[1]];
    int group1_data[dims[0]][dims[1]];
    int group2_data[dims[0]][dims[1]];
    hid_t shared_dataspace_id;
    hid_t group1_dataspace_id;
    hid_t group2_dataspace_id;
    // file
    hid_t file_id;
    char filename[] = "file4.h5";
    // datasets
    hid_t shared_dataset_id;
    char shared_dataset_name[] = "shared";
    hid_t group1_dataset_id;
    char dataset_1_name[] = "some_dataset";
    hid_t group2_dataset_id;
    char dataset_2_name[] = "some_dataset";
    // groups
    hid_t group1_id;
    char group1_name[] = "group1";
    hid_t group2_id;
    char group2_name[] = "group2";
    // attribute
    int age = 81;
    hid_t attribute_id;
    hid_t attribute_dataspace_id;
    hsize_t attribute_dataspace_id_dims = 1;
    int attribute_dataspace_id_rank = 1;

    fill_array(shared_data, dims, rank, 0);
    fill_array(group1_data, dims, rank, -4);
    fill_array(group2_data, dims, rank, 10);
    print_array(shared_data, dims[0]*dims[1]);
    print_array(group1_data, dims[0]*dims[1]);
    print_array(group2_data, dims[0]*dims[1]);

    file_id = H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
        
        group1_id = H5Gcreate(file_id, group1_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            group1_dataspace_id = H5Screate_simple(rank, dims, NULL);
                group1_dataset_id = H5Dcreate(group1_id, dataset_1_name, H5T_STD_I32BE, group1_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                    H5Dwrite(group1_dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, group1_data);
                check_status(H5Dclose(group1_dataset_id));
            check_status(H5Sclose(group1_dataspace_id));
        check_status(H5Gclose(group1_id));
        
         group2_id = H5Gcreate(file_id, group2_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            group2_dataspace_id = H5Screate_simple(rank, dims, NULL);
                group2_dataset_id = H5Dcreate(group2_id, dataset_2_name, H5T_STD_I32BE, group2_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                    H5Dwrite(group2_dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, group2_data);
                check_status(H5Dclose(group2_dataset_id));
            check_status(H5Sclose(group2_dataspace_id));
        check_status(H5Gclose(group2_id));
        

        group1_id = H5Gopen(file_id, group1_name, H5P_DEFAULT);
            //group2_id = H5Gopen(file_id, group2_name, H5P_DEFAULT);
                shared_dataspace_id = H5Screate_simple(rank, dims, NULL);
                    shared_dataset_id = H5Dcreate(group1_id, shared_dataset_name, H5T_STD_I32BE, shared_dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                        H5Dwrite(shared_dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, shared_data);
                        attribute_dataspace_id = H5Screate_simple(1, &attribute_dataspace_id_dims, NULL);
                            attribute_id = H5Acreate(shared_dataset_id, "Age", H5T_STD_I32BE, attribute_dataspace_id, H5P_DEFAULT, H5P_DEFAULT);
                                check_status(H5Awrite(attribute_id, H5T_NATIVE_INT, &age));
                            check_status(H5Aclose(attribute_id));
                        check_status(H5Sclose(attribute_dataspace_id));
                    check_status(H5Dclose(shared_dataset_id));
                check_status(H5Sclose(shared_dataspace_id));
                // todo figure out why it does not work with hardlinks
                //H5Lcreate_hard(shared_dataset_id, shared_dataset_name, group2_id, shared_dataset_name, H5P_DEFAULT, H5P_DEFAULT);
                char sourcename[100];
                char targetname[100];
                snprintf(sourcename, 100, "/%s/%s", group1_name, shared_dataset_name);
                snprintf(targetname, 100, "/%s/%s", group2_name, shared_dataset_name);
                H5Lcreate_soft(sourcename, file_id, targetname, H5P_DEFAULT, H5P_DEFAULT);
            //check_status(H5Gclose(group2_id));
        check_status(H5Gclose(group1_id));

        

    check_status(H5Fclose(file_id));
    exit(0);
}
