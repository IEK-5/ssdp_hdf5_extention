#include <H5Epublic.h>
#include <H5version.h>
#include <hdf5.h>
#include <stdio.h>
#include <stdint.h>
#include <criterion/criterion.h>
#include <stdlib.h>
#include "enums.h"
#include "utils.h"
#include "H5FileIO.h"

static const char* TESTTEMPFILES = "testfiles";
int nrows = 5;
int ncols = 3;
double *data = NULL;

double * make_data(int nrows, int ncols){
    int k = 0;
    double * data = malloc(sizeof(double)*nrows*ncols);
    for(int i = 0; i<nrows; i++){
        for(int j = 0; j<ncols; j++){
            data[i*ncols+j]=(k++);
        }
    }
    return data;
}


static hid_t errorstack;
void suitesetup(){
    /*
        setup callback run prior to each test
        it is independend
        for each test and can be run concorrently
    */
    // turn off error stack for the next function call
    // shared global defaukt errorstack
    H5Eset_auto2(errorstack, NULL, NULL);
}

void suiteteardown(){
    /*
        teardown callback run after each test
        it is independend
        for each test and can be run concorrently
    */
}

TestSuite(H5FileIO, .init=suitesetup, .fini=suiteteardown);

Test(H5FileIO, init_X){
    char *tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(NULL != handler);
    H5FileIOHandler_free(&handler);
    free(tempfile);

    tempfile = make_tempfile(TESTTEMPFILES, true);
    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(NULL == handler);
    H5FileIOHandler_free(&handler);
    free(tempfile);
}

Test(H5FileIO, init_R){
    struct H5FileIOHandler* handler;
    char *tempfile;
    
    tempfile = make_tempfile(TESTTEMPFILES, false);

    handler = H5FileIOHandler_init(tempfile, R);
    cr_assert(NULL == handler);

    handler = H5FileIOHandler_init(tempfile, X);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, R);
    cr_assert(NULL != handler);

    H5FileIOHandler_free(&handler);
    free(tempfile);
}

Test(H5FileIO, init_A){
    struct H5FileIOHandler* handler;
    char *tempfile;
    
    tempfile = make_tempfile(TESTTEMPFILES, false);

    handler = H5FileIOHandler_init(tempfile, A);
    cr_assert(NULL == handler);

    handler = H5FileIOHandler_init(tempfile, X);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, A);
    cr_assert(NULL != handler);

    H5FileIOHandler_free(&handler);
    free(tempfile);
}

Test(H5FileIO, init_W){
    char *tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    cr_assert(NULL != handler);
    free(tempfile);

    tempfile = make_tempfile(TESTTEMPFILES, false);
    handler = H5FileIOHandler_init(tempfile, W);
    cr_assert(NULL != handler);
    H5FileIOHandler_free(&handler);
    free(tempfile);
}

Test(H5FileIO, init_X_exists){
    char *tempfile = make_tempfile(TESTTEMPFILES, false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(NULL != handler);
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(NULL == handler);
    H5FileIOHandler_free(&handler);
    free(tempfile);
}

Test(H5FileIO, init_W_exists){
    char *tempfile = make_tempfile(TESTTEMPFILES,false);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(NULL != handler);
    H5FileIOHandler_free(&handler);
    handler = H5FileIOHandler_init(tempfile, W);
    cr_assert(NULL != handler);
    H5FileIOHandler_free(&handler);
    free(tempfile);
}


Test(H5FileIO, write_array_dataset_does_not_exist){
    char *tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols);
    cr_assert(SUCCESS == err);
    H5FileIOHandler_free(&handler);
    free(data);
}

Test(H5FileIO, write_array_dataset_does_exist){
    char *tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols);
    cr_assert(SUCCESS == err);
    err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols);
    cr_assert(SUCCESS != err);
    H5FileIOHandler_free(&handler);
    free(data);
}

Test(H5FileIO, init_W_overwrites){
    char *tempfile;
    double *read_data;
    int read_nrows;
    int read_ncols;

    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols);
    H5FileIOHandler_free(&handler);

    // test if dataset is still in file
    handler = H5FileIOHandler_init(tempfile, W);
    err = H5FileIOHander_read_array(handler, "some_data", &read_data, &read_ncols, &read_nrows);
    cr_assert(SUCCESS != err);
}  

Test(H5FileIO, read_array_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    char *tempfile;
    
    tempfile = make_tempfile(TESTTEMPFILES,true);
    struct H5FileIOHandler* handler = H5FileIOHandler_init(tempfile, W);
    data = make_data(nrows, ncols);
    ErrorCode err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, ncols);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
   
    err = H5FileIOHander_read_array(handler, "some_data", &read_data, &read_ncols, &read_nrows);
    cr_assert(SUCCESS == err);
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols);
    H5FileIOHandler_free(&handler);

    handler = H5FileIOHandler_init(tempfile, A);
    err = H5FileIOHander_read_array(handler, "some_data", &read_data, &read_ncols, &read_nrows);
    cr_assert(SUCCESS == err);
    cr_assert_arr_eq(read_data, data, sizeof(double)*nrows*ncols);
    H5FileIOHandler_free(&handler);
}

Test(H5FileIO, read_array_does_not_exists){
    double *read_data;
    int read_nrows;
    int read_ncols;
    char *tempfile;
    struct H5FileIOHandler* handler;
    ErrorCode err;

    tempfile = make_tempfile(TESTTEMPFILES,false);

    handler = H5FileIOHandler_init(tempfile, X);
    cr_assert(NULL != handler);
    H5FileIOHandler_free(&handler);
    
    handler = H5FileIOHandler_init(tempfile, R);
    cr_assert(NULL != handler);
    err = H5FileIOHander_read_array(handler, "some_data", &read_data, &read_ncols, &read_nrows);
    cr_assert(SUCCESS != err);
    
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
    make_dir(TESTTEMPFILES);

    struct criterion_test_set *tests = criterion_initialize();

    int result = 0;
    if (criterion_handle_args(argc, argv, true))
        result = !criterion_run_all_tests(tests);

    criterion_finalize(tests);
    /*
        criterion runs test in parallel
        to avoid race conditions clean up shared ressources after the tests are run
    */
    recursive_delete(TESTTEMPFILES);
    return result;   
}
