#pragma once

#include "llvm/Option/OptTable.h"
#include "llvm/Option/ArgList.h"

namespace options {
extern llvm::opt::InputArgList *opts;

enum ClangFlags {
  DriverOption = (1 << 4),
  CoreOption = (1 << 5),
  EmitOption = (1 << 6),
};


enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, \
               HELPTEXT, METAVAR) OPT_##ID,
#include "driver_options.inc"
    LastOption
#undef OPTION
  };
}

class DriverOptTable : public llvm::opt::OptTable
{
public:
    DriverOptTable();
};
