#include "H5Enums.h"
const char* ErrorCode_to_string(ErrorCode err){
    switch (err) {
        case SUCCESS:
            return "SUCCESS";
        case OUTOFMEMORY:
            return "OUTOFMEMORY";
        case FAILURE:
            return "GENERAL FAILURE";
        case DATASET_EXISTS:
            return "DATASET EXISTS";
        default:
            return "UNDEFINED ERROR VALUE";
    }
}
