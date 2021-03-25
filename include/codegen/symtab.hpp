#ifndef _PAREAS_CODEGEN_SYMTAB_HPP
#define _PAREAS_CODEGEN_SYMTAB_HPP

#include "codegen/datatype.hpp"

#include <unordered_map>
#include <cstdint>
#include <string>
#include <iosfwd>

struct Symbol {
    uint32_t id;
    DataType type;
    bool global;
    uint32_t function_offset;
};

class SymbolTable {
    private:
        size_t function_offset = 0;
        size_t max_vars;

        std::unordered_map<std::string, uint32_t> id_map;
        std::unordered_map<std::string, uint32_t> func_id_map;

        uint8_t* data_types;
        bool* globals;
        uint32_t* function_offsets;
    public:
        SymbolTable(size_t);
        ~SymbolTable();

        uint32_t declareSymbol(const std::string&, DataType, bool = false);
        uint32_t declareFunction(const std::string&, DataType);
        Symbol resolveSymbol(const std::string&) const;

        void newFunction();
        void print(std::ostream&) const;

        inline size_t maxVars() const {
            return this->max_vars;
        }
        inline const uint8_t* getDataTypes() const {
            return this->data_types;
        }
        inline const bool* getGlobals() const {
            return this->globals;
        }
        inline const uint32_t* getOffsets() const {
            return this->function_offsets;
        }
};

std::ostream& operator<<(std::ostream&, const SymbolTable&);

#endif
