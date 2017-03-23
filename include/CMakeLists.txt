message(STATUS "Doing TableGen")

include(TableGen)
set(LLVM_TARGET_DEFINITIONS driver_options.td)
set(li1I_TABLEGEN_EXE "${LLVM_TOOLS_BINARY_DIR}/llvm-tblgen")
tablegen(li1I driver_options.inc -gen-opt-parser-defs)
add_public_tablegen_target(DriverOptions)
