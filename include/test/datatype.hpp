#ifndef _PAREAS_TEST_DATATYPE_HPP_
#define _PAREAS_TEST_DATATYPE_HPP_

#include <vector>

enum class DataType {
    INT,
    FLOAT,
    INT_REF,
    FLOAT_REF,
    VOID,
    INVALID
};

const std::vector<DataType> VALUE_TYPES = {DataType::INT, DataType::FLOAT};
const std::vector<DataType> REFERENCE_TYPES = {DataType::INT_REF, DataType::FLOAT_REF};

inline DataType ref_of(DataType t) {
    switch(t) {
        case DataType::INT:
            return DataType::INT_REF;
        case DataType::FLOAT:
            return DataType::FLOAT_REF;
        default:
            return DataType::INVALID;
    }
}

inline DataType value_of(DataType t) {
    switch(t) {
        case DataType::INT_REF:
            return DataType::INT;
        case DataType::FLOAT_REF:
            return DataType::FLOAT;
        default:
            return DataType::INVALID;
    }
}

#endif