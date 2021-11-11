#include "codegen/datatype.hpp"

#include <iostream>

const char* TYPE_NAMES[] = {
    "invalid",
    "void",
    "int",
    "float",
    "int_ref",
    "float_ref"
};


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

DataType value_of(DataType t) {
        switch(t) {
        case DataType::INT_REF:
            return DataType::INT;
        case DataType::FLOAT_REF:
            return DataType::FLOAT;
        default:
            return DataType::INVALID;
    }
}

std::ostream& operator<<(std::ostream& os, const DataType& datatype) {
    os << TYPE_NAMES[static_cast<size_t>(datatype)];
    return os;
}