#include "codegen/datatype.hpp"

DataType reference_of(DataType other) {
    switch(other) {
        case DataType::INT:
            return DataType::INT_REF;
        case DataType::FLOAT:
            return DataType::FLOAT_REF;
        default:
            return DataType::INVALID;
    }
}