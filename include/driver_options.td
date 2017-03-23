// Include the common option parsing interfaces.
include "llvm/Option/OptParser.td"

// Flags
def DriverOption : OptionFlag;
def CoreOption : OptionFlag;
def EmitOption : OptionFlag;
def LinkerInput : OptionFlag;

// Options
def emit_ast : Flag<["--"], "emit-ast">, Flags<[EmitOption]>,
  HelpText<"Emit li1I AST files for source inputs">;
def emit_llvm : Flag<["--"], "emit-llvm">, Flags<[EmitOption]>,
  HelpText<"Emit the LLVM representation for assembler and object files">;
def emit_tokens : Flag<["--"], "emit-tokens">, Flags<[EmitOption]>,
  HelpText<"Emit lexer tokens">;

def o : JoinedOrSeparate<["-"], "o">, Flags<[DriverOption]>,
  HelpText<"Write output to <file>">, MetaVarName<"<file>">;
def e : Flag<["-"], "e">, Flags<[DriverOption]>,
  HelpText<"Execute program using JIT">;
def c : Flag<["-"], "c">, Flags<[DriverOption]>,
  HelpText<"Only compile, don't link">;

def DASH_DASH : Option<["--"], "", KIND_REMAINING_ARGS>,
    Flags<[DriverOption, CoreOption]>;
