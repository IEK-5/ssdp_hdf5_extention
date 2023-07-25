#include <H5Ipublic.h>
#include <H5Tpublic.h>
#include <H5public.h>
#include <hdf5.h>
#include <stdio.h>
#include <stdint.h>
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>
#include <stdlib.h>
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

#include "H5Datatypes.h"
#include "H5Enums.h"
#include "utils.h"
#include "H5FileIO.h"

static const char* TESTTEMPFILES = "testfiles";
char *tempfile;
struct H5FileIOHandler* handler;
ErrorCode err;
time_t tc;
double *read_doubles;
double *written_doubles;
int read_nrows;
int read_ncols;
#define S 1000


#define MAXFRAC 1.9990234375 // (2-1/1024)
double * RandFloatRange16(int n)
{
	double exp, frac;
	double* out = malloc(sizeof(double)*n);
    for(int i = 0; i < n; i++){
        exp = (29.0*(double)rand()/(double)(RAND_MAX) - 14);
	    frac = MAXFRAC*(2*(double)rand()/(double)(RAND_MAX)-1.0);
        out[i] =  frac*pow(2.0,exp);

    }
	return out;
}

double * RandDouble(int n)
{
	double* out = malloc(sizeof(double)*n);
    for(int i = 0; i < n; i++){
        // create random doubles and reject any who are infinite
        union {
            double d;
            unsigned char uc[sizeof(double)];
        } u;
        do {
            for (unsigned i = 0; i < sizeof u.uc; i++) {
            u.uc[i] = (unsigned char) rand();
            }
        } while (!isfinite(u.d));
        out[i] = u.d;

    }
	return out;
}

#define MINHF 6.1035156250000000e-05

#define DFPINF    0x7FF0000000000000 	// positive infinity
#define DFNINF    0xFFF0000000000000 	// negative infinity
#define DFSNAN    0x7FF0000000000001 	// singaling NaN
#define DFQNAN    0x7FF8000000000001 	// quiet NaN
#define DFNAN     0x7FFFFFFFFFFFFFFF 	// NaN
#define HFPINF    0x7C00 	// positive infinity
#define HFNINF    0xFC00 	// negative infinity
#define HFSNAN    0x7C01 	// singaling NaN
#define HFQNAN    0x7E00 	// quiet NaN
#define HFNAN     0x7FFF 	// NaN


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
    srand((unsigned) time(&tc));
}

void suiteteardown(){
    /*
        teardown callback run after each test
        it is independend
        for each test and can be run concurrently
    */
    free(tempfile);
}


TestSuite(H5Datatype, .init=suitesetup, .fini=suiteteardown);

void write_data(char* fn, double *data, int nrows, hid_t dtype){
    handler = H5FileIOHandler_init(fn, IO_W);
    cr_assert(handler != NULL);
    err = H5FileIOHandler_write_array(handler, "some_data", data, nrows, 1, 1, dtype);
    
    cr_assert(SUCCESS == err);
    H5FileIOHandler_free(&handler);
}

void read_data(char *fn){
    handler = H5FileIOHandler_init(fn, IO_R);
    cr_assert(handler != NULL);
    err = H5FileIOHandler_read_array(handler, "some_data", &read_doubles, &read_nrows, &read_ncols);
    cr_assert(SUCCESS == err);
    H5FileIOHandler_free(&handler);
}

Test(H5Datatype, float_16){
    written_doubles = RandFloatRange16(S);
    
    hid_t small_float = H5T_define_16bit_float();
    tempfile = make_tempfile(TESTTEMPFILES, true);
    write_data(tempfile, written_doubles, S, small_float);

    cr_assert(H5Tclose(small_float)>=0);

    read_data(tempfile);
   
    double error;
    const char *limit;
    for(int i = 0; i<S; i++)
    {   
        // 16 bit float should be accurate for these values
        if (fabs(written_doubles[i])>MINHF){
            // we should have full precision
            error=fabs(read_doubles[i]/written_doubles[i]-1);
            limit="no";
        }
        else{// approaching the limits of our float
            error=fabs((read_doubles[i]-written_doubles[i])/MINHF);
            limit="yes";
        }
        // 3 digits are guaranteed, right?
        cr_assert(le(dbl,error,9.765625e-04),"actual=%16lf\texpected=%16lf\tat limit=%s", read_doubles[i], written_doubles[i],limit);
    }
}

Test(H5Datatype, double){
    written_doubles = RandDouble(S);
    tempfile = make_tempfile(TESTTEMPFILES, true);
    write_data(tempfile, written_doubles, S, H5T_NATIVE_DOUBLE);
    read_data(tempfile);
    for(int i = 0; i<S; i++)
    {   
        cr_assert(ieee_ulp_eq(dbl, read_doubles[i], written_doubles[i], 4));
    }
}



Test(H5Datatype, double_special){
    uint64_t written_double_nans[5] = {DFPINF, DFNINF, DFNAN, DFQNAN, DFNAN};
    for(int i = 0; i<5; i++){
        union{
            double f;
            uint64_t i;
        } d1, d2;
        d1.i = written_double_nans[i];
        tempfile = make_tempfile(TESTTEMPFILES, true);
        written_doubles = malloc(sizeof(double));
        written_doubles[0] = d1.f;
        write_data(tempfile, written_doubles, 1, H5T_NATIVE_DOUBLE);
        read_data(tempfile);
        d2.f = read_doubles[0];
        free(written_doubles);
        
        cr_assert(eq(u64, d2.i, d1.i));
    }
}

void test_float_nan(uint64_t nan_value, hid_t float_type){
    // test if all the types of NaNs are correctly represented
    union{
            double f;
            uint64_t i;
        } d1, d2;
    d1.i = nan_value;
    tempfile = make_tempfile(TESTTEMPFILES, true);
    written_doubles = malloc(sizeof(double));
    written_doubles[0] = d1.f;
    write_data(tempfile, written_doubles, 1, float_type);
    read_data(tempfile);
    d2.f = read_doubles[0];
    free(written_doubles);
    cr_assert(eq(u64, d2.i, d1.i), "actual = %#018"PRIx64"\texpected = %#018"PRIx64"\n", d2.i, d1.i);
}

ParameterizedTestParameters(H5Datatype, float_16_special){
    // define a static array of the differnt types of NaNs
    static uint64_t double_nans_params[5] = {DFPINF, DFNINF, DFNAN, DFQNAN, DFNAN};
    size_t nb_params = sizeof(double_nans_params) / sizeof(uint64_t);
    return cr_make_param_array(uint64_t, double_nans_params, nb_params);
}

ParameterizedTest(uint64_t *double_nan, H5Datatype, float_16_special){
    // test the representation and test if it works to 
    hid_t half_float = H5T_define_16bit_float();
    cr_assert(half_float != H5I_INVALID_HID);
    test_float_nan(*double_nan, half_float);
    cr_assert(H5Tclose(half_float)>=0);
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
    if (criterion_handle_args(argc, argv, true)){
        result = !criterion_run_all_tests(tests);
    }
    criterion_finalize(tests);
    /*
        criterion runs test in parallel
        to avoid race conditions clean up shared ressources after the tests are run
    */
    recursive_delete(TESTTEMPFILES, false);
    return result;   
}