#ifndef _PAREAS_CODEGEN_SYMTAB_HPP
#define _PAREAS_CODEGEN_SYMTAB_HPP

#include "codegen/datatype.hpp"

#include <unordered_map>
#include <cstdint>
#include <string>
#include <iosfwd>
#include <vector>

struct Symbol {
    uint32_t id;
    DataType type;
    bool global;
    uint32_t function_offset;
};

static_assert(sizeof(bool) == sizeof(uint8_t));

class SymbolTable {
    private:
        size_t function_offset = 0;
        size_t max_vars;

        std::unordered_map<std::string, uint32_t> id_map;
        std::unordered_map<std::string, uint32_t> func_id_map;

        std::vector<uint8_t> data_types;
        std::vector<uint8_t> globals;
        std::vector<uint32_t> function_offsets;
        std::vector<uint32_t> function_var_count;
        std::vector<std::vector<DataType>> arg_lists;
    public:
        SymbolTable();
        ~SymbolTable();

        uint32_t declareSymbol(const std::string&, DataType, bool = false);
        uint32_t declareFunction(const std::string&, DataType, const std::vector<DataType>&);
        Symbol resolveSymbol(const std::string&) const;

        void newFunction();
        void endFunction();
        void print(std::ostream&) const;

        inline size_t maxVars() const {
            return this->data_types.size();
        }
        inline const uint8_t* getDataTypes() const {
            return this->data_types.data();
        }
        inline const bool* getGlobals() const {
            return reinterpret_cast<const bool*>(this->globals.data());
        }
        inline const uint32_t* getOffsets() const {
            return this->function_offsets.data();
        }
};

std::ostream& operator<<(std::ostream&, const SymbolTable&);

#endif
