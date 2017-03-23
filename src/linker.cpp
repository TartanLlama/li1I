#include <vector>

#include <clang/Driver/Compilation.h>
#include <clang/Driver/Driver.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

#include "linker.hpp"

typedef std::pair<int, const clang::driver::Command*> FailingCommand;

using namespace linker;
using namespace clang;
using namespace clang::driver;
using std::string;

void Linker::link(string input_object, string output_file)
{
    std::string clang_path = llvm::sys::FindProgramByName("clang");

    std::vector<const char *> args;
    args.push_back(clang_path.c_str());
    args.push_back("-o");
    args.push_back(output_file.c_str());
    args.push_back(input_object.c_str());

    DiagnosticOptions *diag_opts = new DiagnosticOptions();
    IntrusiveRefCntPtr<DiagnosticIDs> diag_id(new DiagnosticIDs());
    DiagnosticsEngine diags(diag_id, diag_opts);

    Driver driver(args[0], llvm::sys::getDefaultTargetTriple(), "a.out", diags);

    OwningPtr<Compilation> compilation(driver.BuildCompilation(args));

    int res = 0;
    llvm::SmallVector<std::pair<int, const Command *>, 4> failing_commands;
    if (compilation)
        res = driver.ExecuteCompilation(*compilation, failing_commands);

    for (auto it = failing_commands.begin(); it != failing_commands.end(); it++) {

        int command_res = it->first;
        const Command *failing_command = it->second;
        if (!res)
            res = command_res;

        if (command_res < 0 || command_res == 70) {
            driver.generateCompilationDiagnostics(*compilation, failing_command);
            break;
        }
    }
}
