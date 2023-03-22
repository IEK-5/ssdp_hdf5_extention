#include <hdf5.h>
/*
	This programm creates an empty hdf5 file
*/ 
struct Person {
  char name[50];
  int citNo;
  float salary;
}; 

int main() {
	hid_t file_id;
	herr_t status; 
	file_id = H5Fcreate ("file.h5", H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);   
	status = H5Fclose (file_id);
	struct Person paul;
}
