#include "llvm/IR/LLVMContext.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Assembly/PrintModulePass.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/PluginLoader.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>

#include "bc_compiler.hpp"
#include "driver_options.hpp"
using namespace llvm;
using options::opts;

tool_output_file *BCCompiler::getOutputStream(const std::string &target_name,
                                              Triple::OSType os_type,
                                              const std::string &program_name,
                                              std::string &output_path)
{
    std::string suffix;
    switch (m_file_type) {
    case TargetMachine::CGFT_AssemblyFile:
        if (target_name[0] == 'c') {
            if (target_name[1] == 0)
                suffix = "cbe.c";
            else if (target_name[1] == 'p' && target_name[2] == 'p')
                suffix = "cpp";
            else
                suffix = "s";
        } else
            suffix = "s";
        break;
    case TargetMachine::CGFT_ObjectFile:
        if (os_type == Triple::Win32)
            suffix = "obj";
        else
            suffix = "o";
        break;
    case TargetMachine::CGFT_Null:
        suffix = "null";
        break;
    }

    // Decide if we need "binary" output.
    bool binary = false;
    switch (m_file_type) {
    case TargetMachine::CGFT_AssemblyFile:
        break;
    case TargetMachine::CGFT_ObjectFile:
    case TargetMachine::CGFT_Null:
        binary = true;
        break;
    }

    tool_output_file *out_fd;
    if(opts->hasArg(options::OPT_c))
    {
        std::string error;
        sys::fs::OpenFlags open_flags = sys::fs::F_None;
        if (binary)
            open_flags |= sys::fs::F_Binary;
        std::string output_file = program_name + '.' + suffix;
        output_file = opts->getLastArgValue(options::OPT_o, output_file);
        out_fd = new tool_output_file(output_file.c_str(), error,
                                      open_flags);
        output_path = std::move(output_file);
        
        if (!error.empty()) 
        {
            throw BCCompileError ("Failed to open out fd");
        }
    }
    else
    {
        int fd;
        SmallString<128> result_path;
        llvm::sys::fs::createTemporaryFile (program_name, suffix, fd, result_path);
        out_fd = new tool_output_file(result_path.c_str(), fd);
        output_path = std::move(result_path.c_str());

    }

    return out_fd;
}

std::string BCCompiler::compile(Module *module)
{

    LLVMContext &context = getGlobalContext();

    std::string target_triple (module->getTargetTriple());
    Triple triple (target_triple);

    // Get the target specific parser.
    std::string error;
    const Target *target = TargetRegistry::lookupTarget(MArch, triple,
                                                        error);
    if (!target)
    {
        throw BCCompileError(error);
    }

    // Package up features to be passed to target/subtarget
    std::string FeaturesStr;
    if (MAttrs.size()) {
        SubtargetFeatures Features;
        for (unsigned i = 0; i != MAttrs.size(); ++i)
            Features.AddFeature(MAttrs[i]);
        FeaturesStr = Features.getString();
    }

    TargetOptions options;
    OwningPtr<TargetMachine>
        target_machine(target->createTargetMachine(triple.getTriple(),
                                                  MCPU, FeaturesStr, options,
                                                  RelocModel, CMModel, m_opt_level));
    assert(target_machine.get() && "Could not allocate target machine!");

    // Disable .loc support for older OS X versions.
    if (triple.isMacOSX() &&
        triple.isMacOSXVersionLT(10, 6))
        target_machine->setMCUseLoc(false);

    // Figure out where we are going to send the output.
    std::string output_path;
    OwningPtr<tool_output_file> out
        (getOutputStream(target->getName(), triple.getOS(), module->getModuleIdentifier(), output_path));
    if (!out)
    {
        throw BCCompileError("Cannot open output file");
    }

    // Build up all of the passes that we want to do to the module.
    PassManager pm;

    // Add an appropriate TargetLibraryInfo pass for the module's triple.
    TargetLibraryInfo *tli = new TargetLibraryInfo(triple);
    pm.add(tli);

    // Add intenal analysis passes from the target machine.
    target_machine->addAnalysisPasses(pm);

    // Add the target data from the target machine, if it exists, or the module.
    if (const DataLayout *td = target_machine->getDataLayout())
    {
        pm.add(new DataLayout(*td));
    }
    else
    {
        pm.add(new DataLayout(module));
    }

    // Override default to generate verbose assembly.
    target_machine->setAsmVerbosityDefault(true);
    {
        formatted_raw_ostream fos(out->os());

        // Ask the target to add backend passes as necessary.
        if (target_machine->addPassesToEmitFile(pm, fos, m_file_type))
        {
            throw BCCompileError("Target does not support generation of this file type");
        }

        pm.run(*module);
    }

    out->keep();

    return output_path;
}
