#pragma once

#include <vector>
#include <string>
#include <memory>

#include "boost/iterator/indirect_iterator.hpp"

#include "visitor.hpp"

namespace li1I
{
    enum class Operator
    {
        PLUS, MINUS, TIMES, DIV, EXP, GT, LT, EQ, NEQ
    };

class Program;
class Function;
class VarExpr;
class RPNExpr;
class IntExpr;
class CallExpr;
class DeclExpr;
class OpExpr;
class IfExpr;
class Expr;

    template <typename T>
    using UniqueIterator = 
        boost::indirect_iterator<typename std::vector<std::unique_ptr<T> >::const_iterator>;
    

    using ASTNodeVisitor = Visitor <Program,
                                    Function,
                                    VarExpr,
                                    RPNExpr,
                                    IntExpr,
                                    CallExpr,
                                    DeclExpr,
                                    OpExpr,
                                    IfExpr>;

    class ASTNode : public virtual Visitable<ASTNodeVisitor>
    {
    public:
        virtual ~ASTNode() {}
    };

    template <typename NodeType>
    class VisitableASTNode : public VisitableImpl<NodeType, ASTNodeVisitor>, public virtual ASTNode
    {
    public:
        virtual ~VisitableASTNode() {}
    };

    class Expr : public virtual ASTNode
    {
    public:
        virtual ~Expr() {}
    };

    class RPNExpr : public VisitableASTNode<RPNExpr>
    {
    public:
        RPNExpr();
        inline UniqueIterator<Expr> begin() const { return m_exprs.cbegin(); }
        inline UniqueIterator<Expr> end() const { return m_exprs.cend(); }
    private:
        std::vector<std::unique_ptr<Expr> > m_exprs;
    };

    class IntExpr : public VisitableASTNode<IntExpr>, public Expr
    {
    public:
        IntExpr();
        inline const int &value() const { return m_value; }

    private:
        int m_value;
    };

    class OpExpr : public VisitableASTNode<OpExpr>, public Expr
    {
    public:
        OpExpr();
        inline const Operator &op() const { return m_op; }
    private:
        Operator m_op;
    };

    class VarExpr : public VisitableASTNode<VarExpr>, public Expr
    {
    public:
        VarExpr();
        inline const std::string &vid() const { return m_vid; }
    private:
        std::string m_vid;
    };

    class CallExpr : public VisitableASTNode<CallExpr>, public Expr
    {
    public:
        CallExpr();
        inline const std::string &fid() const { return m_fid; }
    private:
        std::string m_fid;
    };

    class DeclExpr : public VisitableASTNode<DeclExpr>, public Expr
    {
    public:
        DeclExpr();
        inline const std::string &vid() const { return m_vid; }
        inline const RPNExpr &value() const { return *m_value; }
    private:
        std::string m_vid;
        std::unique_ptr<RPNExpr> m_value;
    };

    class IfExpr : public VisitableASTNode<IfExpr>, public Expr
    {
    public:
        IfExpr();
        inline const RPNExpr &condition() const { return *m_condition; }
        inline const RPNExpr &if_forms() const { return *m_if_forms; }
        inline const RPNExpr &else_forms() const { return *m_else_forms; }
    private:
        std::unique_ptr<RPNExpr> m_condition;
        std::unique_ptr<RPNExpr> m_if_forms;
        std::unique_ptr<RPNExpr> m_else_forms;
    };

    class Function : public VisitableASTNode<Function>
    {
    public:
        Function();
        inline const std::string &name() const { return m_name; }
        inline UniqueIterator<VarExpr> begin() const { return m_args.cbegin(); }
        inline UniqueIterator<VarExpr> end() const { return m_args.cend(); }
        inline size_t nArgs() const { return m_args.size(); }
        inline const RPNExpr &expr() const { return *m_expr; }
    private:
        std::string m_name;
        std::vector<std::unique_ptr<VarExpr> > m_args;
        std::unique_ptr<RPNExpr> m_expr;
    };

    class Program : public VisitableASTNode<Program>
    {
    public:
        Program(std::string name);
        inline UniqueIterator<Function> begin() const { return m_functions.cbegin(); }
        inline UniqueIterator<Function> end() const { return m_functions.cend(); }
        inline const std::string &name() const { return m_name; }
    private:
        std::vector<std::unique_ptr<Function> > m_functions;
        std::string m_name;
    };
}
