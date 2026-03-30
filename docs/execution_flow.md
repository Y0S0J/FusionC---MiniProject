# FusionC Execution Flow

This document explains the step-by-step execution path of the FusionC compiler, from the moment a user runs the executable to the final program output or error reporting.

## 1. CLI Entry and Option Parsing (`main.cpp`)
- **Initialization**: The program starts in `main.cpp`.
- **Parsing arguments**: It uses `fusionc::cli::CliParser` to read command-line arguments (e.g., input file, `--dump-tokens`, `--dump-ast`, and the target language).
- **Controller setup**: It instantiates `fusionc::core::CompilerController` and configures `CompilationOptions` based on the user's flags.

## 2. Compilation Pipeline (`core/compiler_controller.cpp`)
The core pipeline is driven by `CompilerController::compileToParser`. It orchestrates the various stages of compilation:

### A. Source Loading & Language Detection
- **Reading**: The input source file is loaded into a string via `readFile`.
- **Detection**: `LanguageLoader::detectLanguage` checks the file extension and content to determine if the language is C (`.c`) or Fusion Custom Language (`.fsc` / `.fcl`).

### B. Frontend: Lexical Analysis
- **Lexer Initialization**: A `frontend::lexer::Lexer` is created using the specific language profile.
- **Tokenization**: `lexer.tokenize(source)` scans the raw string and outputs a stream of `Token` structures, assigning types like identifiers, keywords, and symbols.

### C. Frontend: Parsing
- **Parser Initialization**: A `frontend::parser::Parser` processes the generated tokens.
- **AST Generation**: `parser.parseProgram()` attempts to construct an Abstract Syntax Tree (AST) representing the program's structure based on grammatical rules.
- **Error Collection**: The parser collects any syntax errors encountered (e.g., missing semicolons, unexpected tokens) into a list.

### D. AI-Assisted Error Handling (`ai_module/`)
- If the parser produces **syntax errors**, compilation halts.
- **Enhancement**: Before displaying the errors, `CompilerController` calls `ai::ErrorHandler::buildUserFriendlyErrors`.
- **AI Invocation**: `ErrorHandler` loops through every raw diagnostic. For each error, it calls `AIClient::analyzeSyntaxError`, passing both the raw error string and the entire source code.
- **Command backend**: If `FUSIONC_LLM_CMD_TEMPLATE` is defined (e.g., by `run_with_ai.ps1` using Ollama), a subprocess is spun up to query the local LLM. The LLM's output acts as the user-friendly explanation and suggestion.

### E. Frontend: Semantic Analysis
- If there are no syntax errors, `frontend::semantic::SemanticAnalyzer` traverses the AST.
- It performs scoping, type checking, and validation (e.g., checking if variables are declared before use).

### F. Middle-end: IR Generation & Optimization
- **IR Builder**: The AST is converted into a lower-level Intermediate Representation (IR) via `middleend::ir::buildProgram`.
- **Optimizer**: Basic optimizations are applied, such as `middleend::optimizer::foldConstants(result.ir)`.

### G. Backend: Code Execution
- Through `backend::codegen::execute(result.ir)`, the compiler acts as an interpreter or JIT evaluator, executing the constructed IR.
- The return value or execution state is then mapped back to a program exit code in `main.cpp`.

---

## Why does the LLM sometimes duplicate errors?
When compiling a file with multiple syntax mistakes, the `Parser` may emit multiple raw compiler errors (sometimes cascading from a single typo).
The `ai::ErrorHandler` queries the LLM **independently** for each raw error. Because it feeds the entire source code to the LLM every time, the LLM may fixate on the most obvious error in the file (e.g., a missing semicolon) for *every* query, completely ignoring the specific raw error context. This results in the AI returning the exact same explanation and suggestion for completely different raw compiler diagnostics.
