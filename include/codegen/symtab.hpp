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

        std::vector<std::unordered_map<std::string, uint32_t>> id_map;
        std::unordered_map<std::string, uint32_t> func_id_map;
        std::unordered_map<uint32_t, std::string> rev_id_map;
        std::unordered_map<uint32_t, std::string> rev_func_map;

        std::vector<uint8_t> data_types;
        std::vector<uint8_t> globals;
        std::vector<uint32_t> function_offsets;
        std::vector<uint32_t> function_var_count;
        std::vector<DataType> func_ret_types;
        std::vector<std::vector<DataType>> arg_lists;
    public:
        SymbolTable();
        ~SymbolTable();

        uint32_t declareSymbol(const std::string&, DataType, bool = false);
        uint32_t declareFunction(const std::string&, DataType, const std::vector<DataType>&);
        Symbol resolveSymbol(const std::string&) const;
        uint32_t resolveFunction(const std::string&) const;
        DataType getFunctionReturnType(size_t) const;

        void newFunction();
        void endFunction();
        void print(std::ostream&) const;

        std::string getFunctionName(uint32_t) const;
        std::string getVarName(uint32_t) const;

        inline size_t getCurrentFunction() const {
            return this->func_id_map.size() - 1;
        }

        inline size_t maxVars() const {
            return this->data_types.size();
        }
        inline size_t numFuncs() const {
            return this->function_var_count.size();
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
        inline const uint32_t* getFuncVarCount() const {
            return this->function_var_count.data();
        }

        void enterScope();
        void exitScope();
};

std::ostream& operator<<(std::ostream&, const SymbolTable&);

#endif
