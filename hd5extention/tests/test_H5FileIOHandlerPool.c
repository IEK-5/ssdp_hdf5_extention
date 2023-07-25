#include <H5Fpublic.h>
#include <H5Ipublic.h>
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

struct H5FileIOHandlerPool *pool;
const char* TESTDIRROOT = "testfiles";
const char* TESTDIR = "testfiles/handlertestfiles";

void suitesetup(){
    /*
        setup callback run prior to each test
        it is independend
        for each test and can be run concurrently
    */
    pool = H5FileIOHandlerPool_init();
}

void suiteteardown(){
    /*
        teardown callback run after each test
        it is independend
        for each test and can be run concurrently
    */
    H5FileIOHandlerPool_free(&pool);
}

TestSuite(H5FileIOHandlerPool, .init=suitesetup, .fini=suiteteardown);

Test(H5FileIOHandlerPool, get_handler_twice){
    // calling twice with the same arguments get should give you a valid handler
    char *fn = make_tempfile(TESTDIR, false);
    struct H5FileIOHandler *handler1 = H5FileIOHandlerPool_get_handler(pool, fn, IO_A);
    struct H5FileIOHandler *handler2 = H5FileIOHandlerPool_get_handler(pool, fn, IO_A);
    cr_assert(handler1 != NULL, "Handler not created\n");
    cr_assert(handler1 == handler2, "Two different handlers created\n");

    // second call with different mode must result in NULL
    struct H5FileIOHandler *handler3 = H5FileIOHandlerPool_get_handler(pool, fn, IO_R);
    cr_assert(handler3 == NULL, "Handler not created\n");
    free(fn);
    

}

ErrorCode write_double(struct H5FileIOHandler* handler, double val){
    double data[] = {val};
    return H5FileIOHandler_write_array(handler, "data", data, 1, 1, 1, H5T_NATIVE_DOUBLE);
}

ErrorCode read_double(struct H5FileIOHandler* handler, double **val){
    
    int out_nrows;
    int out_ncols;
    ErrorCode err = H5FileIOHandler_read_array(handler, "data", val, &out_nrows, &out_ncols);
    cr_assert(out_nrows == 1, "Handler did not write data correctly: rows");
    cr_assert(out_ncols == 1, "Handler did not write data correctly: columns");
    return err;
}

Test(H5FileIOHandlerPool, get_handler_pool_full){
    // test if increasing the poolsize works correctly
    const int n = pool->poolsize*3;
    char **files = malloc(sizeof(char*)*n);
    for(int i = 0; i < n; i++){
        char *file = make_tempfile(TESTDIR, false);
        files[i]=file;
        struct H5FileIOHandler* handler = H5FileIOHandlerPool_get_handler(pool, file, IO_W);
        cr_assert(handler != NULL, "Handler Creation failed");
        cr_assert(write_double(handler, i) == SUCCESS);
        
    }

    H5FileIOHandlerPool_close_all_files(pool);

    for(int i = 0; i < n; i++){
        double *read = NULL;
        struct H5FileIOHandler* handler = H5FileIOHandler_init(files[i], IO_R);
        ErrorCode err = read_double(handler, &read);
        cr_assert(err == SUCCESS);
        cr_assert(ieee_ulp_eq(dbl, *read, (double) i, 4), "Handler did not write data correctly");
        free(files[i]);
    }
    free(files);
}

Test(H5FileIOHandlerPool, close_file){
    // test if when we close a file that it not in our pool anymore 
    // this test is kinda all over the place...
    // TODO check if there is a way to test by hdf5 if a file is open I did not find any such test. the H5Iis_valid function doesnt
    // work as a released resource is considered valid

    double expected = 3.14;
    double *actual;
    ErrorCode err;
    //fileno uniquely identify an open file see https://docs.hdfgroup.org/hdf5/develop/group___h5_f.html#ga402205688af065ab5db0fe20417d5484
    unsigned long fileno1, fileno2, fileno3;
    hid_t fileid1, fileid2, fileid3;

    // create the file
    struct H5FileIOHandler *handler1, *handler2, *handler3;
    char *file = make_tempfile(TESTDIR, false);
    handler1 = H5FileIOHandlerPool_get_handler(pool, file, IO_X);
    fileid1 = handler1->file_id;
    cr_assert(H5Fget_fileno(fileid1, &fileno1)>=0);
    
    cr_assert(write_double(handler1, 3.14) == SUCCESS);
    H5FileIOHandlerPool_close_file(pool, file);
    cr_assert(pool->handlers[0] == NULL);

    // read it once
    handler2 = H5FileIOHandlerPool_get_handler(pool, file, IO_R);
    fileid2 = handler2->file_id;
    cr_assert(H5Fget_fileno(fileid2, &fileno2)>=0);
    // only the first slot in the pool shoudl be used
    cr_assert(pool->handlers[0] != NULL);
    cr_assert(pool->handlers[1] == NULL);
    err = read_double(handler2, &actual);
    cr_assert(err == SUCCESS);
    cr_assert(ieee_ulp_eq(dbl, *actual, expected, 4), "Handler did not write data correctly");
    H5FileIOHandlerPool_close_file(pool, file);

    // read again
    handler3 = H5FileIOHandlerPool_get_handler(pool, file, IO_R);
    fileid3 = handler3->file_id;
    cr_assert(H5Fget_fileno(fileid3, &fileno3)>=0);
    // only the first slot in the pool shoudl be used
    cr_assert(pool->handlers[0] != NULL);
    cr_assert(pool->handlers[1] == NULL);
    err = read_double(handler3, &actual);
    cr_assert(err == SUCCESS);
    cr_assert(ieee_ulp_eq(dbl, *actual, expected, 4), "Handler did not write data correctly");
    H5FileIOHandlerPool_close_file(pool, file);
    free(file);

    cr_assert(fileno1!=fileno2, "Duplicate Fileno");
    cr_assert(fileno2!=fileno3, "Duplicate Fileno");
    cr_assert(fileno1!=fileno3, "Duplicate Fileno");
}

Test(H5FileIOHandlerPool, close_all_files){
    
    int n = pool->poolsize;
    char **files = malloc(sizeof(char*)*n);
    for(int i = 0; i < n; i++){
        char *file = make_tempfile(TESTDIR, false);
        H5FileIOHandlerPool_get_handler(pool, file, IO_X);
        files[i]=file;
    }
    H5FileIOHandlerPool_close_all_files(pool);
    for(int i = 0; i < n; i++){
        cr_assert(pool->handlers[i] == NULL, "Handler#%d not closed!", i);
        free(files[i]);
    }
    free(files);
    
}

Test(H5FileIOHandlerPool, get_handler_pool_gap){
    // test if a gap filled by a close gets correctly filled
    int n;
    char **files;
    int rand_idx;
    double expected = 17.0;
    double *actual;

    n = pool->poolsize;
    files = malloc(sizeof(char*)*n);
    rand_idx = rand() % n;

    for(int i = 0; i < n; i++){
        char *file = make_tempfile(TESTDIR, false);
        files[i] = file;
        struct H5FileIOHandler *handler = H5FileIOHandlerPool_get_handler(pool, file, IO_X);
        cr_assert(handler != NULL, "Error while creating handler");
        if (i == rand_idx){
            write_double(handler, expected);
        }
    }

    H5FileIOHandlerPool_close_file(pool, files[rand_idx]);
    cr_assert(pool->handlers[rand_idx] == NULL, "Handler not closed!");
    
    struct H5FileIOHandler *handler = H5FileIOHandlerPool_get_handler(pool, files[rand_idx], IO_R);
    cr_assert(handler != NULL, "Error while creating handler");
    cr_assert(pool->handlers[rand_idx] != NULL, "No new handler created in pool");
    read_double(handler, &actual);
    cr_assert(ieee_ulp_eq(dbl, *actual, expected, 4), "Handler did not write data correctly");

    for(int i = 0; i < n; i++){
        free(files[i]);
    }
    free(files);
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
    srand(time(NULL));
    recursive_delete(TESTDIR, false);
    make_dir(TESTDIRROOT);
    make_dir(TESTDIR);

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