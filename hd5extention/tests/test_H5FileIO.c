#include <hdf5.h>
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
char *tempfile;
int nrows = 5;
int ncols = 3;
double *data = NULL;
char const ** columns_names;
static hid_t errorstack;
H5E_auto2_t oldfunc;
void *old_client_data;

double * make_data(int nrows, int ncols){
    /*
        Fill the array with random integer values which are smaler than UINT16_MAX/10
        in this range we do not lose any digits which makes testing easier
    */
    
    double * data = malloc(sizeof(double)*nrows*ncols);
    for(int i = 0; i<nrows; i++){
        for(int j = 0; j<ncols; j++){
            data[i*ncols+j]=i*ncols+j;//(rand())%(UINT16_MAX/10);
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
    
    data = make_data(nrows, ncols);
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
    free(data);
    free(columns_names);
    free(tempfile);
}


TestSuite(H5FileIO, .init=suitesetup, .fini=suiteteardown);

Test(H5FileIO, init_X){
    tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    free(tempfile);

    tempfile = make_tempfile(TESTTEMPFILES, true);
    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler == NULL, "Handler creation should fail");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_R){
    struct H5FileIOHandler* handler;
    
    tempfile = make_tempfile(TESTTEMPFILES, false);

    handler = H5FileIOHandler_init(tempfile, R);
    cr_assert(handler == NULL, "Handler creation failed");

    handler = H5FileIOHandler_init(tempfile, X);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, R);
    cr_assert(handler != NULL, "Handler creation should fail");

    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_A){
    struct H5FileIOHandler* handler;
    
    tempfile = make_tempfile(TESTTEMPFILES, false);
    handler = H5FileIOHandler_init(tempfile, A);
    cr_assert(handler != NULL, "Handler creation should never fail due to filestyetem reasons with A");

    handler = H5FileIOHandler_init(tempfile, X);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, A);
    cr_assert(handler != NULL, "Handler creation failed");

    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_W){
    tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    cr_assert(handler != NULL, "Handler creation failed");
    free(tempfile);

    tempfile = make_tempfile(TESTTEMPFILES, false);
    handler = H5FileIOHandler_init(tempfile, W);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_X_exists){
    tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler == NULL, "Handler creation should fail");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_W_exists){
    tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, W);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
}


Test(H5FileIO, write_array_dataset_does_not_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Writing array failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_array_dataset_does_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Writing array failed %s", ErrorCode_to_string(err));
    err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS != err, "Writing array worked but it should not %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_table_dataset_does_not_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data, nrows, ncols, columns_names, 1);
    cr_assert(SUCCESS == err, "Writing table failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, write_table_dataset_does_exist){
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data, nrows, ncols, columns_names, 1);
    err = H5FileIOHandler_write_table(handler, "some_data", data, nrows, ncols, columns_names, 1);
    cr_assert(SUCCESS != err, "Writing table failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, init_W_overwrites){
    double *read_data;
    int read_nrows;
    int read_ncols;

    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    H5FileIOHandler_free(&handler);

    // test if dataset is still in file
    handler = H5FileIOHandler_init(tempfile, W);
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS != err, "Writing array should fail %s", ErrorCode_to_string(err));
}  

Test(H5FileIO, read_array_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols, 1, H5T_NATIVE_DOUBLE);
    cr_assert(SUCCESS == err, "Writing arrays failed %s", ErrorCode_to_string(err));
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
   
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS == err, "Reading array failed %s", ErrorCode_to_string(err));
    cr_assert(read_nrows == nrows, "Wrong read nrows");
    cr_assert(read_ncols == ncols, "Wrong read ncols");
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols, "Data should be exactly the same");
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, A);
    err = H5FileIOHandler_read_array(handler, "some_data", &read_data, &read_nrows, &read_ncols);
    cr_assert(SUCCESS == err, "Reading array failed %s", ErrorCode_to_string(err));
    cr_assert(read_nrows == nrows, "Wrong read nrows");
    cr_assert(read_ncols == ncols, "Wrong read ncols");
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols, "Data should be exactly the same");
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_array_does_not_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    struct H5FileIOHandler* handler;
    ErrorCode err;

    tempfile = make_tempfile(TESTTEMPFILES,false);

    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler != NULL, "Handler creation failed");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
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
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data, nrows, ncols, columns_names, 1);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
   
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols, "Read values are not written values");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, A);
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols, "Read values are not written values");
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
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    data = make_data(big_nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_table(handler, "some_data", data, big_nrows, ncols, columns_names, 1000);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
   
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
     cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols, "Read values are not written values");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, A);
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
     cr_assert(SUCCESS == err, "Reading of table failed %s", ErrorCode_to_string(err));
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols, "Read values are not written values");
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

    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(handler != NULL, "Creation of Handler failed");
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
    cr_assert(handler != NULL, "Creation of Handler failed");
    err = H5FileIOHandler_read_table(handler, "some_data", &read_data, &read_nrows, &read_ncols, &read_columnnames);
    cr_assert(SUCCESS != err, "Reading of table should fail %s", ErrorCode_to_string(err));

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
    recursive_delete(TESTTEMPFILES);
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
