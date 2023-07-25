#include <H5Tpublic.h>
#include <hdf5.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

#include "H5Datatypes.h"
#include "H5Enums.h"
#include "utils.h"
#include "H5FileIO.h"

static const char* TESTTEMPFILES = "testfiles";
char *tempfile = NULL;
int nrows = 5;
int ncols = 3;
double *data_contiguous = NULL;
double **data_array_of_columns = NULL;
char const ** columns_names = NULL;
static hid_t errorstack;
H5E_auto2_t oldfunc;
void *old_client_data = NULL;

double * make_data_contiguous(int nrows, int ncols){
    /*
        Fill the array with random integer values which are smaler than UINT16_MAX/10
        in this range we do not lose any digits which makes testing easier
    */
    
    double * data = malloc(sizeof(double)*nrows*ncols);
    for(int i = 0; i<nrows; i++){
        for(int j = 0; j<ncols; j++){
            data[i*ncols+j]=i*ncols+j;
        }
    }
    return data;
}

double ** make_data_array_of_columns(int nrows, int ncols){
    /*
        Fill the array with random integer values which are smaler than UINT16_MAX/10
        in this range we do not lose any digits which makes testing easier
    */
    
    double ** data = malloc(sizeof(double)*ncols*sizeof(double*));
    for(int i = 0; i<ncols; i++){
        data[i] = malloc(sizeof(double *)*nrows);
        for(int j = 0; j<nrows; j++){
            data[i][j]=i%ncols+j*ncols;
        }
    }
    return data;
}

void suitesetup(){
    /*
        setup callback run prior to each test
        it is independend
        for each test and can be run concurrently
    */
    
    // shared global default errorstack
    // turn off error stack for the next function call
    // save old error handler
    //H5Eget_auto2(errorstack, &oldfunc, &old_client_data);
    // turn of errorhandling
    H5Eset_auto2(errorstack, NULL, NULL);
    
    data_contiguous = make_data_contiguous(nrows, ncols);
    data_array_of_columns = make_data_array_of_columns(nrows, ncols);
    columns_names = malloc(sizeof(char *)*3);
    columns_names[0] = "A";
    columns_names[1] = "B";
    columns_names[2] = "C";
    
}

void suiteteardown(){
    /*
        teardown callback run after each test
        it is independend
        for each test and can be run concurrently
    */
    free(data_contiguous);
    for(int i = 0; i < ncols; i++){
        free(data_array_of_columns[i]);
    }
    free(data_array_of_columns);
    free(columns_names);
    free(tempfile);
}


TestSuite(H5FileIO, .init=suitesetup, .fini=suiteteardown);

Test(H5FileIO, init_X){
    tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    free(tempfile);

    tempfile = make_tempfile(TESTTEMPFILES, true);
    handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler == NULL, "Handler creation should fail");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_R){
    struct H5FileIOHandler* handler;
    
    tempfile = make_tempfile(TESTTEMPFILES, false);

    handler = H5FileIOHandler_init(tempfile, IO_R);
    cr_assert(handler == NULL, "Handler creation failed");

    handler = H5FileIOHandler_init(tempfile, IO_X);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, IO_R);
    cr_assert(handler != NULL, "Handler creation should fail");

    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_A){
    struct H5FileIOHandler* handler;
    
    tempfile = make_tempfile(TESTTEMPFILES, false);
    handler = H5FileIOHandler_init(tempfile, IO_A);
    cr_assert(handler != NULL, "Handler creation should never fail due to filestyetem reasons with A");

    handler = H5FileIOHandler_init(tempfile, IO_X);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, IO_A);
    cr_assert(handler != NULL, "Handler creation failed");

    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_W){
    tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    cr_assert(handler != NULL, "Handler creation failed");
    free(tempfile);

    tempfile = make_tempfile(TESTTEMPFILES, false);
    handler = H5FileIOHandler_init(tempfile, IO_W);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_X_exists){
    tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler == NULL, "Handler creation should fail");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_W_exists){
    tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, IO_W);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
}


Test(H5FileIO, write_array_dataset_does_not_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Writing array failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_array_dataset_does_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Writing array failed %s", ErrorCode_to_string(err));
    err = H5FileIOHandler_write_array(handler, "some_data", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS != err, "Writing array worked but it should not %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_table_dataset_does_not_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data_contiguous, nrows, ncols, columns_names, 1);
    cr_assert(SUCCESS == err, "Writing table failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_table_dataset_does_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data_contiguous, nrows, ncols, columns_names, 1);
    err = H5FileIOHandler_write_table(handler, "some_data", data_contiguous, nrows, ncols, columns_names, 1);
    cr_assert(SUCCESS != err, "Writing table failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_W_overwrites){
    double *read_data;
    int read_nrows;
    int read_ncols;

    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    H5FileIOHandler_free(&handler);

    // test if dataset is still in file
    handler = H5FileIOHandler_init(tempfile, IO_W);
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS != err, "Writing array should fail %s", ErrorCode_to_string(err));
}  

Test(H5FileIO, read_array_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_W);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Writing arrays failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_R);
   
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS == err, "Reading array failed %s", ErrorCode_to_string(err));
    cr_assert(read_nrows == nrows, "Wrong read nrows");
    cr_assert(read_ncols == ncols, "Wrong read ncols");
    cr_assert_arr_eq(read_data, data_contiguous, sizeof(double)*nrows*ncols, "Data should be exactly the same");
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, IO_A);
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS == err, "Reading array failed %s", ErrorCode_to_string(err));
    cr_assert(read_nrows == nrows, "Wrong read nrows");
    cr_assert(read_ncols == ncols, "Wrong read ncols");
    cr_assert_arr_eq(read_data, data_contiguous, sizeof(double)*nrows*ncols, "Data should be exactly the same");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_array_does_not_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    struct H5FileIOHandler* handler;
    ErrorCode err;

    tempfile = make_tempfile(TESTTEMPFILES,false);

    handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_R);
    cr_assert(handler != NULL, "Handler creation failed");
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS != err, "Reading of array should fail %s", ErrorCode_to_string(err));

    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_table_exists){
    // turn on error handling
    // H5Eset_auto2(errorstack, oldfunc, old_client_data);
    double *read_data;
    int read_nrows;
    int read_ncols;
    char **read_columnnames;
    tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_X);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data_contiguous, nrows, ncols, columns_names, 1);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_R);
   
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data_contiguous, sizeof(double)*nrows*ncols, "Read values are not written values");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_A);
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data_contiguous, sizeof(double)*nrows*ncols, "Read values are not written values");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_write_big_table){
    // turn on error handling
    // H5Eset_auto2(errorstack, oldfunc, old_client_data);
    double *read_data;
    int read_nrows;
    int read_ncols;
    int big_nrows = 60000;
    char **read_columnnames;
    tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, IO_X);
    data_contiguous = make_data_contiguous(big_nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data_contiguous, big_nrows, ncols, columns_names, 1000);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_R);
   
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data_contiguous, sizeof(double)*nrows*ncols, "Read values are not written values");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_A);
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data_contiguous, sizeof(double)*nrows*ncols, "Read values are not written values");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_table_does_not_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    char **read_columnnames;
    struct H5FileIOHandler* handler;
    ErrorCode err;

    tempfile = make_tempfile(TESTTEMPFILES,false);

    handler = H5FileIOHandler_init(tempfile, IO_X);
    cr_assert(handler != NULL, "Creation of Handler failed");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_R);
    cr_assert(handler != NULL, "Creation of Handler failed");
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS != err, "Reading of table should fail %s", ErrorCode_to_string(err));

    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, test_read_write_array_IO_MODE_A){
    double *read_data;
    int read_nrows;
    int read_ncols;
    struct H5FileIOHandler* handler;
    ErrorCode err;


    tempfile = make_tempfile(TESTTEMPFILES,false);
    // write some data
    handler = H5FileIOHandler_init(tempfile, IO_X);
    err = H5FileIOHandler_write_array(handler, "d1", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Related file: %s\n",tempfile);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_A);
    err = H5FileIOHandler_write_array(handler, "d2", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Related file: %s\n",tempfile);
    err = H5FileIOHandler_read_array(handler, "d1", &read_data, &read_ncols, &read_nrows);
    cr_assert(SUCCESS == err, "Related file: %s\n",tempfile);
    for(int i = 0; i < ncols*nrows; i++){
        cr_assert(eq(dbl,data_contiguous[i],read_data[i]), "Mismatch in element %d: written:%lf read:%ls");
    }
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, test_read_write_table_IO_MODE_A){
    double *read_data;
    int read_nrows;
    int read_ncols;
    struct H5FileIOHandler* handler;
    char **read_columnnames;
    ErrorCode err;


    tempfile = make_tempfile(TESTTEMPFILES,false);
    // write some data
    handler = H5FileIOHandler_init(tempfile, IO_X);
    err = H5FileIOHandler_write_table(handler, "d1", data_contiguous, nrows, ncols, columns_names, 1);
    cr_assert(SUCCESS == err, "Related file: %s\n",tempfile);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, IO_A);
    err = H5FileIOHandler_write_table(handler, "d2", data_contiguous, nrows, ncols, columns_names, 1);
    cr_assert(SUCCESS == err, "Related file: %s\n",tempfile);
    err = H5FileIOHandler_read_table(handler, "d1", &read_data, &read_ncols, &read_nrows, &read_columnnames);
    cr_assert(SUCCESS == err, "Related file: %s\n",tempfile);
    for(int i = 0; i < ncols*nrows; i++){
        cr_assert(eq(dbl,data_contiguous[i],read_data[i]), "Mismatch in element %d: written:%lf read:%ls");
    }
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_array_of_columns){
    ErrorCode err;
    tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler *handler = H5FileIOHandler_init(tempfile, IO_X);
    err = H5FileIOHandler_write_array(handler, "contiguous", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(err==SUCCESS);
    err=H5FileIOHandler_write_array_of_columns(handler, "array_of_pointers", data_array_of_columns, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(err==SUCCESS);
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, IO_R);
    double *read1, *read2;
    int read_ncols1, read_nrows1, read_ncols2, read_nrows2;;
    err=H5FileIOHandler_read_array(handler, "contiguous", &read1, &read_nrows1, &read_ncols1);
    cr_assert(err==SUCCESS);
    err=H5FileIOHandler_read_array(handler, "array_of_pointers", &read2, &read_nrows2, &read_ncols2);
    cr_assert(err==SUCCESS);
    cr_assert(eq(int,read_ncols1, read_ncols2), "Mismatch in number of columns! cont:%d vs arr of ptr:%d", read_ncols1, read_ncols2);
    cr_assert(eq(int,read_nrows1, read_nrows2), "Mismatch in number of rows! cont:%d vs arr of ptr:%d", read_nrows1, read_nrows2);
    for(int i = 0; i < ncols*nrows; i++){
        cr_assert(eq(dbl,read1[i],read2[i]), "Mismatch in element %d: written:%lf read1:%ls read2:%ls", i, data_contiguous[i], read1[i], read2[i]);
    }
    
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_array_of_columns){
    tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler *handler = H5FileIOHandler_init(tempfile, IO_X);
    H5FileIOHandler_write_array(handler, "data", data_contiguous, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, IO_R);
    double *read_contiguous, **read_array_of_columns;
    int read_ncols1, read_nrows1, read_ncols2, read_nrows2;;
    cr_assert(H5FileIOHandler_read_array(handler, "data", &read_contiguous, &read_nrows1, &read_ncols1)==SUCCESS);
    cr_assert(H5FileIOHandler_read_array_of_columns(handler, "data", &read_array_of_columns, &read_nrows2, &read_ncols2)==SUCCESS);
    cr_assert(eq(int,read_ncols1, read_ncols2, ncols), "Mismatch in number of columns! cont:%d vs arr of ptr:%d", read_ncols1, read_ncols2);
    cr_assert(eq(int,read_nrows1, read_nrows2, nrows), "Mismatch in number of rows! cont:%d vs arr of ptr:%d", read_nrows1, read_nrows2);
    for(int col = 0; col < ncols; col++){
        for(int row = 0; row < nrows; row++){
            cr_assert(eq(dbl,read_contiguous[row*ncols+col],read_array_of_columns[col][row]), "Mismatch in element col:%d row:%d: written:%lf read1:%ls read2:%ls", 
            col, row, data_contiguous[row*ncols+col],read_contiguous[col*nrows+row],read_array_of_columns[col][row]);
        }
    }
    free(read_contiguous);
    for(int col = 0; col < ncols; col++){
        free(read_array_of_columns[col]);
    }
    free(read_array_of_columns);
    H5FileIOHandler_free(&handler);
}

/* This is necessary on windows, as BoxFort needs the main to be exported
   in order to find it. */
#if defined (_WIN32) || defined (__CYGWIN__)
# if defined (_MSC_VER)
#  define DLLEXPORT __declspec(dllexport)
# elif defined (__GNUC__)
#  define DLLEXPORT __attribute__((dllexport))
# else
#  error No dllexport attribute
# endif
#else
# define DLLEXPORT
#endif


DLLEXPORT int main(int argc, char *argv[]) {
    /*
        criterion runs test in parallel
        to avoid race conditions create sgared ressoures prior to the tests running
    */
    srand(17);
    recursive_delete(TESTTEMPFILES, false);
    make_dir(TESTTEMPFILES);

    struct criterion_test_set *tests = criterion_initialize();

    int result = 0;
    if (criterion_handle_args(argc, argv, true)){
        result = !criterion_run_all_tests(tests);
    }
    criterion_finalize(tests);
    /*
        criterion runs test in parallel
        to avoid race conditions clean up shared ressources after the tests are run
    */
    return result;   
}
