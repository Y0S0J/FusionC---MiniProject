# FusionC Guided Walkthrough (One Full Program Through Every Stage)

This document maps compiler theory to your current implementation, step by step, using one real input.

## 1) Input Program (C)

Source used: `tests/test_c/sample.c`

```c
int main()
{
  int x = 10;
  int y = x + 5;
  y = y * 2;
  return y;
}
```

Expected behavior in this project: program exit code should be `30`.

---

## 2) Stage-by-Stage: Theory -> Your Code

### Stage A: Lexical Analysis (Tokenizer)

Theory:
- Converts raw characters into tokens (keywords, identifiers, operators, punctuation, etc.).

Your code:
- `frontend/lexer/token.cpp`

Real token output from your current binary:

```text
[Keyword] 'int'
[Identifier] 'main'
[Punctuation] '('
[Punctuation] ')'
[Punctuation] '{'
[Keyword] 'int'
[Identifier] 'x'
[Operator] '='
[Number] '10'
[Punctuation] ';'
[Keyword] 'int'
[Identifier] 'y'
[Operator] '='
[Identifier] 'x'
[Operator] '+'
[Number] '5'
[Punctuation] ';'
[Identifier] 'y'
[Operator] '='
[Identifier] 'y'
[Operator] '*'
[Number] '2'
[Punctuation] ';'
[Keyword] 'return'
[Identifier] 'y'
[Punctuation] ';'
[Punctuation] '}'
[EndOfFile] '<eof>'
```

---

### Stage B: Syntax Analysis (Parser -> AST)

Theory:
- Checks grammar rules and builds a tree (AST) that represents program structure.

Your code:
- `frontend/parser/ast.cpp`

Real AST output from your current binary:

```text
Program(root)
  Function(main:int)
    Block(block)
      ExpressionStatement(stmt)
        Declaration(int)
          Identifier(x)
          Literal(10)
      ExpressionStatement(stmt)
        Declaration(int)
          Identifier(y)
          BinaryExpression(+)
            Identifier(x)
            Literal(5)
      ExpressionStatement(stmt)
        Assignment(=)
          Identifier(y)
          BinaryExpression(*)
            Identifier(y)
            Literal(2)
      Return(return)
        Identifier(y)
```

Interpretation:
- One function: `main` with return type `int`.
- Body contains declaration, declaration with expression, assignment, and return.

---

### Stage C: AI-Assisted Syntax Error Handling

Theory:
- When syntax diagnostics are produced, an NLP-style helper can translate terse compiler errors into beginner-friendly explanations and likely fixes.
- If AI is unavailable, the compiler should still return standard diagnostics.

Your code:
- `ai_module/ai_client.cpp`
- `ai_module/error_handler.cpp`
- Wired in `core/compiler_controller.cpp`

What happens in current implementation:
- Parser and lexer errors are collected first.
- If errors exist before semantic analysis, the controller invokes `ErrorHandler`.
- Syntax diagnostics are transformed to include:
  - `Why:` explanation
  - `Suggestion:` corrective action
- If `FUSIONC_DISABLE_AI=1`, the same errors are passed through unchanged.

Example transformed error shape:

```text
Syntax error: Expected ';' at end of statement.
    Why: The statement is incomplete because the parser reached the end of a statement without a semicolon.
    Suggestion: Add ';' at the end of the statement, usually after an expression, declaration, or function call.
```

---

### Stage D: Semantic Analysis

Theory:
- Validates meaning beyond syntax: scope, undeclared variables, duplicate declarations, type checks.

Your code:
- `frontend/semantic/symbol_table.cpp`

What it checks now:
- Duplicate declarations in the same scope.
- Use of undeclared identifiers.
- Return type consistency against function "declared return type".

For this C sample:
- `x` and `y` are declared before use.
- Return expression type resolves to `int`.
- No semantic errors.

---

### Stage E: Intermediate Representation (TAC)

Theory:
- Lowers AST into simple instructions (three-address code) that are easy to optimize and execute.

Your code:
- `middleend/ir/three_address_code.cpp`

Generated TAC for this sample (derived from builder logic):

```text
copy x, 10
add t0, x, 5
copy y, t0
mul t1, y, 2
copy y, t1
ret y
```

Interpretation:
- Temporary variables (`t0`, `t1`) hold intermediate expression results.

---

### Stage F: Optimization (Constant Folding)

Theory:
- Rewrites expressions at compile time when values are known constants.

Your code:
- `middleend/optimizer/optimizer.cpp`

Current optimization behavior:
- Folds only arithmetic instructions where both args are numeric literals.

For this sample:
- No change (because instructions use variables like `x` and `y`, not just literals).

Optimized TAC remains:

```text
copy x, 10
add t0, x, 5
copy y, t0
mul t1, y, 2
copy y, t1
ret y
```

---

### Stage G: Backend Execution (Current "Codegen")

Theory:
- Final stage usually emits machine code/object code.

Your current code:
- `backend/codegen/machine_codegen.cpp`

What it actually does now:
- Interprets TAC in-memory (a tiny virtual machine style execution).
- Returns exit code when `ret` is hit.

For this sample:
- Exit code is `30`.

---

## 3) What a Mini Compiler Does vs Full Production Compiler

## What your mini compiler DOES
- End-to-end compile pipeline flow (lex -> parse -> semantic -> IR -> optimize -> execute).
- Basic language support for arithmetic/declarations/assignments/return.
- Produces useful diagnostics for many beginner mistakes.
- Great for learning compiler internals.

## What it DOES NOT do yet (typical production features)
- No native machine code generation (x86/ARM assembly/object files).
- No linker integration for multi-file programs.
- No complete C language support (no robust control flow lowering, function params/calls model, pointers, structs, preprocessor behavior, etc.).
- No advanced optimizations (CFG, SSA, DCE, CSE, inlining, register allocation).
- No full error-recovery and rich diagnostics infrastructure.

Short summary:
- This is a real educational compiler pipeline.
- It is not yet a full C compiler toolchain like `clang`/`gcc`.

---

## 4) Current Custom Language Status

Current validation status with the project binary:

1. `tests/test_custom/sample.fsc` compiles and runs with `fn main() { ... }`.
2. `tests/test_custom/print.fsc` compiles and runs, printing integer output.
3. `tests/test_custom/print_var.fsc` compiles and runs, printing string + variable output.
4. Both explicit `custom` hint and extension-based auto-detection work for these files.

Observed runtime behavior:
- `sample.fsc` prints `30` and exits with code `0`.
- `print.fsc` prints `20` and exits with code `0`.
- `print_var.fsc` prints `The answer is:` then `42`, and exits with code `0`.

---

## 5) Code Map: "Where to Read What"

- Program entry and output printing: `main.cpp`
- Pipeline orchestration: `core/compiler_controller.cpp`
- Language detection and profiles: `core/language_loader.cpp`
- CLI parsing: `fusionc_cli/cli_parser.cpp`
- Lexer: `frontend/lexer/token.cpp`
- Parser + AST stringify: `frontend/parser/ast.cpp`
- Semantic analysis + symbol table: `frontend/semantic/symbol_table.cpp`
- IR (TAC) building: `middleend/ir/three_address_code.cpp`
- Optimizer (constant folding): `middleend/optimizer/optimizer.cpp`
- TAC interpreter backend: `backend/codegen/machine_codegen.cpp`

---

## 6) If You Want to Grow This Toward a Full Compiler

Suggested order:
1. Add control flow (`if`, `while`) to parser + IR with labels/jumps.
2. Add function parameters and calls.
3. Expand type system and conversions.
4. Replace interpreter backend with assembly/LLVM emission.
5. Add stage-wise tests (lexer/parser/semantic/IR/optimizer/backend).

This order gives fast learning and visible progress.
