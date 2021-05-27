#include <vector>
#include <memory>

#include <llvm/Support/Program.h>

#include "linker.hpp"

using std::string;

void li1I::Linker::link(string input_object, string output_file)
{
    throw std::runtime_exception{"Not implemented yet, use your system linker"};
}
