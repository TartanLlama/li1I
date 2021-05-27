cmake_minimum_required (VERSION 2.8.7)
project (li1I)

find_package(LLVM)
find_package(Boost)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DEFINITIONS}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
link_directories(${LLVM_LIBRARY_DIRS})
set(LLVM_MAIN_INCLUDE_DIR ${LLVM_INCLUDE_DIRS})
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

file(GLOB_RECURSE li1I_sources "src/*.cpp")

include_directories("include" "${CMAKE_CURRENT_BINARY_DIR}/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -frtti -g")

add_subdirectory(include)

add_executable(li1I ${li1I_sources})
add_dependencies(li1I DriverOptions)

set (LIBS
    LLVMOption
    LLVMInterpreter
    LLVMX86Disassembler
    LLVMX86CodeGen
    LLVMX86Desc
    LLVMSelectionDAG
    LLVMObject
    LLVMAsmPrinter
    LLVMX86AsmParser
    LLVMMCParser
    LLVMX86Info
    LLVMX86Utils
    LLVMMCJIT
    LLVMExecutionEngine
    LLVMCodeGen
    LLVMScalarOpts
    LLVMInstCombine
    LLVMTransformUtils
    LLVMAnalysis
    LLVMTarget
    LLVMMC
    LLVMCore
    LLVMSupport
    Boost::boost
    pthread
    dl
    curses
)

target_link_libraries (li1I ${LIBS})
