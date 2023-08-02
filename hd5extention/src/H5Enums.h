#pragma once
typedef enum {
    IO_X, // create a file and fail if the file does exists
    IO_W, // create a file and if the file does exists overwrite it
    IO_A, // open an existing file in ReadWrite mode if it does not exist create it
    IO_R, // open an existing file in ReadOnly mode if it does not exist fail
} IOMode;

// Maybe use H5E_values ?
typedef enum {
    SUCCESS = 0,
    FAILURE = -1,
    WRONG_IO_MODE = -2,
    OUTOFMEMORY = -3,
    DATASET_EXISTS = -4,
    // add others with different negative values
} ErrorCode;

const char* ErrorCode_to_string(ErrorCode err);
