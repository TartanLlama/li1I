#include "llvm/Option/Option.h"
#include "llvm/ADT/STLExtras.h"
#include "driver_options.hpp"

using namespace options;
using namespace llvm::opt;

#define PREFIX(NAME, VALUE) static const char *const NAME[] = VALUE;
#include "driver_options.inc"
#undef PREFIX

static const OptTable::Info InfoTable[] = {
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, ALIASARGS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)   \
  { PREFIX, NAME, HELPTEXT, METAVAR, OPT_##ID, Option::KIND##Class, PARAM, \
    FLAGS, OPT_##GROUP, OPT_##ALIAS, ALIASARGS },
#include "driver_options.inc"
#undef OPTION
};

DriverOptTable::DriverOptTable()
    : OptTable(InfoTable, llvm::array_lengthof(InfoTable)) {}

