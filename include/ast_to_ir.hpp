#pragma once

#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>

#include "ast.hpp"

namespace li1I
{
    class IRTransformError : public std::exception
    {
    public:
IRTransformError (std::string message) : m_message(message) {}
        ~IRTransformError() throw() {}

        virtual const char* what() const throw()
        {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };


    class ASTToIRVisitor : public ASTNodeVisitor
    {
    public:
        ASTToIRVisitor() :
            m_module(NULL), m_builder(llvm::getGlobalContext()), m_environment()
        {}
        void visit(const Program &node);
        void visit(const Function &node);
        void visit(const VarExpr &node);
        void visit(const RPNExpr &node);
        void visit(const IntExpr &node);
        void visit(const CallExpr &node);
        void visit(const DeclExpr &node);
        void visit(const OpExpr &node);
        void visit(const IfExpr &node);
        llvm::Module *codegenIR(const Program &program);

    private:
        llvm::Value *codegen(const ASTNode &node);
        llvm::Value *codegenOperation (Operator op,
                                           llvm::Value *lhs, llvm::Value *rhs);
        void createMain();

        llvm::Module *m_module;
        llvm::IRBuilder<> m_builder;
        std::map <std::string, llvm::Value*> m_environment;
        llvm::Value *m_value;
    };
}
