#include "ast_dumper.hpp"
#include "ast.hpp"

using namespace li1I;
using std::string;
using std::ostream;
using std::endl;

static const int indentation_amount = 4;

void ASTDumper::dumpNode(const ASTNode &node)
{
    m_level++;
    output("(", false);
    node.accept(this);
    output(")", true);
    m_level--;
}

string ASTDumper::pad()
{
    return string(m_level*indentation_amount, ' ');
}

void ASTDumper::output(string str, bool newline)
{
    static bool add_line = false;

    if (add_line && str.compare(")") != 0)
    {
        *m_out << endl;
        *m_out << pad();
    }

    add_line = newline;

    *m_out << str;
}

ASTDumper::ASTDumper(std::ostream *os, const Program &ast) : m_level(-1), m_out(os)
{
    dumpNode(ast);
}

void ASTDumper::visit(const Program &node)
{
    output("Program");

    for (auto &function : node)
    {
        dumpNode(function);
    }
}

void ASTDumper::visit(const Function &node)
{
    output("Function ", false);
    output(node.name());

    for (auto &arg : node)
    {
        dumpNode(arg);
    }

    dumpNode(node.expr());
}

void ASTDumper::visit(const VarExpr &node)
{
    output("VarExpr ", false);
    output(node.vid(), false);
}

void ASTDumper::visit(const RPNExpr &node)
{
    output("RPNExpr");
    for (auto &expr : node)
    {
        dumpNode(expr);
    }
}

void ASTDumper::visit(const IntExpr &node)
{
    output("IntExpr ", false);
    output(std::to_string(node.value()), false);
}

void ASTDumper::visit(const CallExpr &node)
{
    output("CallExpr ", false);
    output(node.fid());
}

void ASTDumper::visit(const DeclExpr &node)
{
    output("DeclExpr ", false);
    output(node.vid());
    dumpNode(node.value());
}

void ASTDumper::visit(const OpExpr &node)
{
    output("OpExpr ", false);

    string op;
    switch(node.op())
    {
    case Operator::PLUS: op = "PLUS"; break;
    case Operator::MINUS: op = "MINUS"; break;
    case Operator::TIMES: op = "TIMES"; break;
    case Operator::DIV: op = "DIV"; break;
    case Operator::EXP: op = "EXP"; break;
    case Operator::GT: op = "GT"; break;
    case Operator::LT: op = "LT"; break;
    case Operator::EQ: op = "EQ"; break;
    case Operator::NEQ: op = "NEQ"; break;
    }

    output(op, false);
}


void ASTDumper::visit(const IfExpr &node)
{
    output("IFExpr");
    dumpNode (node.condition());
    dumpNode (node.if_forms());
    dumpNode (node.else_forms());
}
