#include <fstream>
#include <cstdio>
#include <vector>
#include <iostream>
#include <sstream>
#include <iterator>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/TargetSelect.h>
#include "llvm/Support/TargetRegistry.h"
#include <llvm/ADT/Triple.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/Host.h>
#include "llvm/Option/ArgList.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"

#include "lexer.hpp"
#include "parser.hpp"
#include "ast_dumper.hpp"
#include "ast_to_ir.hpp"
#include "bc_compiler.hpp"
#include "linker.hpp"
#include "driver_options.hpp"

llvm::opt::InputArgList *options::opts;
using options::opts;
using namespace li1I;

int main(int argc, char **argv)
{
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();    

    llvm::llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

    llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);

    // Enable debug stream buffering.
    llvm::EnableDebugBuffering = true;
    // Register the target printer for --version.
    llvm::cl::AddExtraVersionPrinter(llvm::TargetRegistry::printRegisteredTargetsForVersion);

    DriverOptTable opt_table;
    unsigned missing_arg_index;
    unsigned missing_arg_count;

    llvm::ArrayRef<char*> argv_ref(argv, argc);
    opts = new llvm::opt::InputArgList{opt_table.ParseArgs(argv_ref, missing_arg_index, missing_arg_count)};
    opts->ClaimAllArgs();
    std::string in_filename = opts->getLastArgValue(options::OPT_INPUT);
    std::ifstream program(in_filename);
    std::string program_name = llvm::sys::path::stem(in_filename);
    std::string object_path(in_filename);
    bool object_file_is_temp = false;

    if (llvm::sys::path::extension(in_filename).equals(".li"))
    {
        Lexer lexer(program);
        Parser parser;
        Program *ast = parser.parse(&lexer, &program, program_name);

        if (opts->hasArg(options::OPT_emit_ast))
        {
            ASTDumper dumper (&std::cout, *ast);
        }

        ASTToIRVisitor codegenner;
        std::unique_ptr<llvm::Module> module {codegenner.codegenIR(*ast)};
        delete ast;
    
        if (opts->hasArg(options::OPT_emit_llvm))
        {
            module->print(llvm::errs(), nullptr);
        }

        if (opts->hasArg(options::OPT_e))
        {
            llvm::ExecutionEngine *ee;
            llvm::Function *main_function = module->getFunction(llvm::StringRef("IIII"));
            ee = llvm::EngineBuilder(std::move(module)).create();
            std::vector<llvm::GenericValue> args;
            llvm::GenericValue result = ee->runFunction(main_function, args);
            std::cout << std::endl << *result.IntVal.getRawData() << std::endl;
            return 0;
        }

        BCCompiler bc_compiler (llvm::CodeGenOpt::Level::None,
                                llvm::CodeGenFileType::CGFT_ObjectFile);
        object_path = bc_compiler.compile(module.get());
        object_file_is_temp = true;
        }
    else if (llvm::sys::path::extension(in_filename).equals(".o"))
    {
        object_file_is_temp = false;
    }
    
    if (!opts->hasArg(options::OPT_c))
    {
        li1I::Linker linker;
        linker.link(object_path, opts->getLastArgValue(options::OPT_o, "a.out"));
        if (object_file_is_temp)
        {
            std::remove(object_path.c_str());
        }
    }
    return 0;
}
