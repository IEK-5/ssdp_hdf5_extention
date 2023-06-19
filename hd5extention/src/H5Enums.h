#pragma once
typedef enum {
    X, // create a file and fail if the file does exists
    W, // create a file and if the file does exists overwrite it
    A, // open an existing file in ReadWrite mode if it does not exist create it
    R, // open an existing file in ReadOnly mode if it does not exist fail
} IOMode;

// Maybe use H5E_values ?
typedef enum {
    SUCCESS = 0,
    OUTOFMEMORY = 1,
    FAILURE = -1,
    WRONG_IO_MODE = -2,
    // add others with different negative values
} ErrorCode;

const char* ErrorCode_to_string(ErrorCode err);