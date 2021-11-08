#ifndef _PAREAS_CONVERTER_CONVERTER_HPP
#define _PAREAS_CONVERTER_CONVERTER_HPP

class ASTNode;
class SymbolTable;

#include "codegen/datatype.hpp"

#include <iosfwd>

namespace pareas {
    class SourceConverter {
        private:
            std::ostream& os;
            const SymbolTable* symtab;
            size_t indent = 0;

            void printIndent();
            void printDataType(DataType);
            
            inline void enterScope() {
                ++this->indent;
            }

            void exitScope() {
                --this->indent;
            }

            void convertStatementList(const ASTNode*);
            void convertFunctionDecl(const ASTNode*);
            void convertArgList(const ASTNode*);
            void convertArgDecl(const ASTNode*);
            void convertBinaryOp(const ASTNode*);
            void convertUnaryOp(const ASTNode*);
            void convertAssignOp(const ASTNode*);
            void convertLiteral(const ASTNode*);
            void convertCast(const ASTNode*);
            void convertDecl(const ASTNode*);
            void convertReturn(const ASTNode*);
            void convertIf(const ASTNode*);
            void convertIfElse(const ASTNode*);
            void convertWhile(const ASTNode*);
            void convertFuncCall(const ASTNode*);
        public:
            SourceConverter(std::ostream&, const SymbolTable*);
            ~SourceConverter() = default;

            void convert(const ASTNode*);
    };
};

#endif
