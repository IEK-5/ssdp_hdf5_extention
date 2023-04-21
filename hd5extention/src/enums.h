#pragma once
typedef enum {
    CREATE,
    WRITE,
    READ,
} IOMode;

// Maybe use H5E_values ?
typedef enum {
    SUCCESS = 0,
    OUTOFMEMORY = 1,
    FAILURE = -1,
    // add others with different negative values
} ErrorCode;

const char* ErrorCode_to_string(ErrorCode err);