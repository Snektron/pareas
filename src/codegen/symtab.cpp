#include "codegen/symtab.hpp"
#include "codegen/exception.hpp"

SymbolTable::SymbolTable() {
}

SymbolTable::~SymbolTable() {
}

uint32_t SymbolTable::declareSymbol(const std::string& name, DataType type, bool global) {
    auto& last = this->id_map.back();
    if(last.count(name) > 0)
        throw ParseException("Redeclaration of symbol ", name);
    uint32_t id = this->data_types.size();
    last[name] = id;

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

    this->func_ret_types.push_back(type);
    this->arg_lists.push_back(arg_types);

    return id;
}

Symbol SymbolTable::resolveSymbol(const std::string& name) const {
    uint32_t id;
    bool found = false;
    for(size_t i = this->id_map.size(); i > 0; --i) {
        const auto& elem = this->id_map.at(i-1);
        if(elem.count(name) > 0) {
            id = elem.at(name);
            found = true;
            break;
        }
    }

    if(!found)
        throw ParseException("Use of undeclared symbol ", name);

    return Symbol{
        .id = id,
        .type = static_cast<DataType>(this->data_types[id]),
        .global = (bool)this->globals[id],
        .function_offset = this->function_offsets[id]
    };
}

uint32_t SymbolTable::resolveFunction(const std::string& name) const {
    if(this->func_id_map.count(name) == 0)
        throw ParseException("Tried to call undeclared function ", name);
    return this->func_id_map.at(name);
}

DataType SymbolTable::getFunctionReturnType(size_t id) const {
    return this->func_ret_types[id];
}

void SymbolTable::newFunction() {
    this->function_offset = 0;
}

void SymbolTable::endFunction() {
    this->function_var_count.push_back(this->function_offset);
}

void SymbolTable::enterScope() {
    this->id_map.push_back({});
}

void SymbolTable::exitScope() {
    this->id_map.pop_back();
}

void SymbolTable::print(std::ostream& os) const {
    for(size_t i = 0; i < this->data_types.size(); ++i) {
        os << "symbol " << i << " -> (" << static_cast<DataType>(this->data_types[i])
                     << ", " << this->function_offsets[i] << ", " << this->globals[i] << ")" << std::endl;
    }
}

std::ostream& operator<<(std::ostream& os, const SymbolTable& tab) {
    tab.print(os);
    return os;
}