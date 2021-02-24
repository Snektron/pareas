#include "codegen/symtab.hpp"
#include "codegen/exception.hpp"

SymbolTable::~SymbolTable() {
    for(auto& del : this->symbols)
        delete del.second;
}

uint32_t SymbolTable::declareSymbol(const std::string& name, DataType type) {
    if(this->symbols.count(name) > 0)
        throw ParseException("Redeclaration of symbol ", name);
    uint32_t id = this->symbols.size();
    this->symbols[name] = new Symbol;
    this->symbols[name]->id = id;
    this->symbols[name]->type = type;
    return id;
}

Symbol* SymbolTable::resolveSymbol(const std::string& name) const {
    if(this->symbols.count(name) == 0)
        throw ParseException("Use of undeclared symbol ", name);
    return this->symbols.at(name);
}