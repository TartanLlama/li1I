#pragma once

#include "llvm/Target/TargetMachine.h"
#include "llvm/ADT/Triple.h"

namespace llvm
{
    class ToolOutputFile;
    class Module;
    class LLVMContext;
}

class BCCompileError : public std::exception
{
public:
    BCCompileError (std::string message) : m_message(message) {}
    ~BCCompileError() throw() {}
    virtual const char* what() const throw()
    {
        return m_message.c_str();
    }
private:
    std::string m_message;
};


class BCCompiler
{
public:
    BCCompiler (llvm::CodeGenOpt::Level opt_level,
                llvm::CodeGenFileType file_type)
        : m_opt_level(opt_level), m_file_type(file_type) {};
    std::string compile(llvm::Module *module);
private:
    llvm::CodeGenOpt::Level m_opt_level;
    llvm::CodeGenFileType m_file_type;

    llvm::ToolOutputFile *getOutputStream(const std::string &target_name,
                                            llvm::Triple::OSType os_type,
                                            const std::string &prog_name,
                                            std::string &output_path);
};
