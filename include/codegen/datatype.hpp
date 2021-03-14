#ifndef _PAREAS_CODEGEN_DATATYPE_HPP
#define _PAREAS_CODEGEN_DATATYPE_HPP

#include <iosfwd>

enum class DataType {
    INVALID,
    VOID,
    INT,
    FLOAT,
    INT_REF,
    FLOAT_REF
};

DataType reference_of(DataType);

std::ostream& operator<<(std::ostream&, const DataType&);

#endif
