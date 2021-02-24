#ifndef _PAREAS_CODEGEN_SYMTAB_HPP
#define _PAREAS_CODEGEN_SYMTAB_HPP

#include "codegen/datatype.hpp"

#include <unordered_map>
#include <cstdint>
#include <string>

struct Symbol {
    uint32_t id;
    DataType type;
};

class SymbolTable {
    private:
        std::unordered_map<std::string, Symbol*> symbols;
    public:
        ~SymbolTable();

        uint32_t declareSymbol(const std::string&, DataType);
        Symbol* resolveSymbol(const std::string&) const;
};

#endif
