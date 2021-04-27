#include "codegen/symtab.hpp"
#include "codegen/exception.hpp"

SymbolTable::SymbolTable() {
}

SymbolTable::~SymbolTable() {
}

uint32_t SymbolTable::declareSymbol(const std::string& name, DataType type, bool global) {
    if(this->id_map.count(name) > 0)
        throw ParseException("Redeclaration of symbol ", name);
    uint32_t id = this->id_map.size();
    this->id_map[name] = id;

    this->data_types.push_back(static_cast<uint8_t>(type));
    this->globals.push_back(global);
    this->function_offsets.push_back(this->function_offset++);

    return id;
}

uint32_t SymbolTable::declareFunction(const std::string& name, DataType type, const std::vector<DataType>& arg_types) {
    if(this->func_id_map.count(name) > 0)
        throw ParseException("Redeclaration of function ", name);
    uint32_t id = this->func_id_map.size();
    this->func_id_map[name] = id;

    this->arg_lists.push_back(arg_types);

    return id;
}

Symbol SymbolTable::resolveSymbol(const std::string& name) const {
    if(this->id_map.count(name) == 0)
        throw ParseException("Use of undeclared symbol ", name);
    uint32_t id = this->id_map.at(name);

    return Symbol{
        .id = id,
        .type = static_cast<DataType>(this->data_types[id]),
        .global = (bool)this->globals[id],
        .function_offset = this->function_offsets[id]
    };
}

void SymbolTable::newFunction() {
    this->function_offset = 0;
}

void SymbolTable::endFunction() {
    this->function_var_count.push_back(this->function_offset);
}

void SymbolTable::print(std::ostream& os) const {
    for(auto& symb : this->id_map) {
        os << symb.first << " -> (" << symb.second << ", " << static_cast<DataType>(this->data_types[symb.second])
                     << ", " << this->function_offsets[symb.second] << ", " << this->globals[symb.second] << ")" << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const SymbolTable& tab) {
    tab.print(os);
    return os;
}