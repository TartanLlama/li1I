#pragma once

#include <string>
#include <iostream>

#include "ast.hpp"

namespace li1I
{
    class ASTDumper : public ASTNodeVisitor
    {
    public:
        ASTDumper(std::ostream *os, const Program &ast);
        void dumpNode(const ASTNode &node);
        void visit(const Program &node);
        void visit(const Function &node);
        void visit(const VarExpr &node);
        void visit(const RPNExpr &node);
        void visit(const IntExpr &node);
        void visit(const CallExpr &node);
        void visit(const DeclExpr &node);
        void visit(const OpExpr &node);
        void visit(const IfExpr &node);
    private:
        int m_level;
        std::ostream *m_out;
        std::string pad();
        void output(std::string str, bool newline=true);
    };
}
