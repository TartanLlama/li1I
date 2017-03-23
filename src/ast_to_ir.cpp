#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Host.h>
#include <sstream>
#include <stack>
#include <iostream>
#include <memory>

#include "ast_to_ir.hpp"

using namespace li1I;
using llvm::ConstantInt;
using llvm::IntegerType;
using llvm::FunctionType;
using llvm::BasicBlock;
using llvm::Type;

void ASTToIRVisitor::createMain()
{
    FunctionType *ft = FunctionType::get(llvm::Type::getInt32Ty(m_context),
                                                     false);
    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "main", m_module);
    
    BasicBlock *entry = BasicBlock::Create(m_context, "entry", f);
    m_builder.SetInsertPoint(entry);
    llvm::Function *callee = m_module->getFunction("IIII");

    llvm::ArrayRef<Type*> args (llvm::Type::getInt8PtrTy(m_context));
    ft = FunctionType::get(llvm::Type::getInt32Ty(m_context),
                           args,
                           true);
    f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, "printf", m_module);
    m_builder.CreateCall(f, {m_builder.CreateGlobalStringPtr("%d\n"), m_builder.CreateCall(callee)});
    
    m_builder.CreateRet(ConstantInt::get(IntegerType::get(m_context,32), llvm::APInt(32, 0, true)));
}

void ASTToIRVisitor::visit(const Program &node)
{
    m_module = new llvm::Module(node.name(), m_context);

    for (auto &func : node)
    {
       func.accept(this);
    }

    createMain();
}

void ASTToIRVisitor::visit(const Function &node)
{
    m_environment.clear();

    std::vector<llvm::Type*> arg_types (node.nArgs(),
                                        llvm::Type::getInt32Ty(m_context));
    llvm::FunctionType *ft = llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context),
                                         arg_types, false);

    llvm::Function *f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, node.name(), m_module);

    if (f->getName() != node.name())
    {
        throw IRTransformError("Function redefinition");
    }

    auto p_arg = node.begin();
    for (auto f_arg = f->arg_begin(); p_arg != node.end();
         ++f_arg, ++p_arg)
    {
        f_arg->setName(p_arg->vid());
        m_environment[p_arg->vid()] = static_cast<llvm::Argument*>(f_arg);
    }

    llvm::BasicBlock *entry = llvm::BasicBlock::Create(m_context, "entry", f);
    m_builder.SetInsertPoint(entry);

    llvm::Value *ret;
    if ((ret = codegen(node.expr())))
    {
        m_builder.CreateRet(ret);
    }

    llvm::verifyFunction(*f);
}

void ASTToIRVisitor::visit(const VarExpr &node)
{
    m_value = m_environment[node.vid()];

    if (!m_value)
    {
        std::stringstream ss;
        ss << "No such variable as " << node.vid();
        throw IRTransformError(ss.str());
    }
}

llvm::Value *ASTToIRVisitor::codegenOperation (Operator op, llvm::Value *lhs, llvm::Value *rhs)
{
    llvm::Value *v;
    switch (op)
    {
    case Operator::PLUS: v = m_builder.CreateAdd(lhs, rhs); break;
    case Operator::MINUS: v = m_builder.CreateSub(lhs, rhs); break;
    case Operator::TIMES: v = m_builder.CreateMul(lhs, rhs); break;
    case Operator::DIV: v = m_builder.CreateUDiv(lhs, rhs); break;
    case Operator::EXP: v = m_builder.CreateAdd(lhs, rhs); break; //TODO
    case Operator::GT: v = m_builder.CreateICmpUGT(lhs, rhs); break;
    case Operator::LT: v = m_builder.CreateICmpULT(lhs, rhs); break;
    case Operator::EQ: v = m_builder.CreateICmpEQ(lhs, rhs); break;
    case Operator::NEQ: v = m_builder.CreateICmpNE(lhs, rhs); break;
    }

    v = m_builder.CreateIntCast(v, llvm::IntegerType::get(m_context, 32), true);

    return v;
}

void ASTToIRVisitor::visit(const RPNExpr &node)
{
    std::stack<llvm::Value*> rpn_stack;
    for (auto &expr : node)
    {
        const OpExpr *op = dynamic_cast<const OpExpr*>(&expr);
        const CallExpr *call = dynamic_cast<const CallExpr*>(&expr);
        if (op)
        {
            if (rpn_stack.size() < 2)
            {
                throw IRTransformError("Not enough items on stack");
            }

            llvm::Value *rhs = rpn_stack.top();
            rpn_stack.pop();

            llvm::Value *lhs = rpn_stack.top();
            rpn_stack.pop();

            rpn_stack.push(codegenOperation(op->op(), lhs, rhs));
        }
        else if(call)
        {
            llvm::Function *callee = m_module->getFunction(call->fid());
            if (rpn_stack.size() < callee->arg_size())
            {
                throw IRTransformError("Not enough items on stack to call function");
            }

            std::vector<llvm::Value*> arg_values;
            for (int i = 0; i < callee->arg_size(); i++)
            {
                arg_values.push_back(rpn_stack.top());
                rpn_stack.pop();
            }

            rpn_stack.push(m_builder.CreateCall(callee, arg_values));
        }
        else
        {
            rpn_stack.push(codegen(expr));
        }
    }

    if (rpn_stack.size() > 1)
    {
        throw IRTransformError("Too many items on stack after RPN expression");
    }

    if (rpn_stack.size() == 0)
    {
        throw IRTransformError("No items on stack after RPN expression");
    }

    m_value = rpn_stack.top();
}

void ASTToIRVisitor::visit(const CallExpr &node)
{
    throw IRTransformError("CallExprs shouldn't be visited");
}

void ASTToIRVisitor::visit(const DeclExpr &node)
{
    llvm::Value *value = codegen(node.value());
    m_environment[node.vid()] = value;
    m_value = value;
}

void ASTToIRVisitor::visit(const OpExpr &node)
{
    throw IRTransformError("OpExprs shouldn't be visited");
}

void ASTToIRVisitor::visit (const IfExpr &node)
{
    llvm::Value *cond = codegen(node.condition());

    m_builder.CreateICmpNE(cond,
                           llvm::ConstantInt::get(llvm::IntegerType::get(m_context,32), llvm::APInt(32, 0, true)),
                         "ifcond");

    llvm::Function *fun = m_builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *then_block = llvm::BasicBlock::Create(m_context, "then", fun);
    llvm::BasicBlock *else_block = llvm::BasicBlock::Create(m_context, "else");
    llvm::BasicBlock *merge_block = llvm::BasicBlock::Create(m_context, "ifcont");

    llvm::Value *br_cond = m_builder.CreateIntCast(cond, llvm::IntegerType::get(m_context, 1), false);
    m_builder.CreateCondBr(br_cond, then_block, else_block);

    m_builder.SetInsertPoint(then_block);

    llvm::Value *then_value = codegen(node.if_forms());

    m_builder.CreateBr(merge_block);
    then_block = m_builder.GetInsertBlock();

    fun->getBasicBlockList().push_back(else_block);
    m_builder.SetInsertPoint(else_block);

    llvm::Value *else_value = codegen(node.else_forms());

    m_builder.CreateBr(merge_block);
    else_block = m_builder.GetInsertBlock();

    fun->getBasicBlockList().push_back(merge_block);
    m_builder.SetInsertPoint(merge_block);
    llvm::PHINode *phi = m_builder.CreatePHI(llvm::Type::getInt32Ty(m_context), 2,
                                    "iftmp");

    phi->addIncoming(then_value, then_block);
    phi->addIncoming(else_value, else_block);

    m_value = phi;
}

void ASTToIRVisitor::visit(const IntExpr &node)
{
    m_value = llvm::ConstantInt::get(m_context, llvm::APInt(32, node.value(), true));
}

llvm::Value *ASTToIRVisitor::codegen (const ASTNode &node)
{
    node.accept(this);
    llvm::Value *value = m_value;
    m_value = NULL;
    return value;
}

llvm::Module *ASTToIRVisitor::codegenIR(const Program &program)
{
    program.accept(this);
    m_module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    return m_module;
}
