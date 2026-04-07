# FusionC - Intelligent Multi-Language Compiler

FusionC is a teaching-friendly compiler pipeline that can parse, semantically check, lower to three-address code, run a constant-folding optimizer, and interpret the program. Current support covers a subset of C (single function, declarations, assignments, arithmetic, returns) and the provided CustomLang sample grammar.

## Capabilities

- Language detection (extension + optional hint)
- Lexical analysis ([frontend/lexer/token.cpp](frontend/lexer/token.cpp))
- Parsing to AST with functions/blocks/returns ([frontend/parser/ast.cpp](frontend/parser/ast.cpp))
- AI-assisted syntax error explanations and corrective suggestions with fallback to standard errors ([ai_module/error_handler.cpp](ai_module/error_handler.cpp))
- Semantic checks with scoped symbols and basic return-type validation ([frontend/semantic/symbol_table.cpp](frontend/semantic/symbol_table.cpp))
- TAC generation ([middleend/ir/three_address_code.cpp](middleend/ir/three_address_code.cpp))
- Constant folding optimizer ([middleend/optimizer/optimizer.cpp](middleend/optimizer/optimizer.cpp))
- TAC interpreter backend ([backend/codegen/machine_codegen.cpp](backend/codegen/machine_codegen.cpp))

## Architecture Pipeline
1) **Language detection** — [core/language_loader.cpp](core/language_loader.cpp)
2) **Lexing** — produces tokens
3) **Parsing** — builds AST with functions/blocks/expressions
4) **AI-assisted syntax diagnostics** — transforms syntax/unknown-token errors into friendly explanations and suggestions when AI is available
5) **Semantic analysis** — symbol table, duplicate/undefined checks, return type consistency
6) **IR build** — three-address code (const, copy, add, sub, mul, div, ret)
7) **Optimization** — constant folding
8) **Execution** — TAC interpreter; reports exit code

## Project Layout (key files)

- Entry: [main.cpp](main.cpp)
- CLI: [fusionc_cli/cli_parser.cpp](fusionc_cli/cli_parser.cpp)
- Controller: [core/compiler_controller.cpp](core/compiler_controller.cpp)
- AI diagnostics: [ai_module/ai_client.cpp](ai_module/ai_client.cpp), [ai_module/error_handler.cpp](ai_module/error_handler.cpp)
- Frontend: lexer/parser/semantic under `frontend/`
- Middle-end: TAC + optimizer under `middleend/`
- Backend: interpreter under `backend/codegen/`
- Samples: [tests/test_c/sample.c](tests/test_c/sample.c), [tests/test_custom/sample.fsc](tests/test_custom/sample.fsc)

## AI-Assisted Error Handling
- Syntax-oriented compiler errors (for example, `Expected ';'`, `Unexpected token`, `Unknown token`) are post-processed into user-friendly messages.
- Enhanced messages include:
	- `Why`: plain-language explanation of what failed.
	- `Suggestion`: likely corrective action.
- If the AI component is unavailable, FusionC automatically falls back to the original compiler diagnostics.

Optional LLM backend (command-based):
- Set `FUSIONC_LLM_CMD_TEMPLATE` to a shell command that accepts the compiler error text and source code. The placeholders `{error}` and `{code}` are replaced accordingly.
- Example with Ollama (use single quotes for the environment variable so prompt contents are cleanly passed):

```powershell
$env:FUSIONC_LLM_CMD_TEMPLATE='ollama run llama3:8b-instruct-q4_K_M "You are an expert compiler assistant. Output exactly two lines: `Explanation: <short explanation>` and `Suggestion: <how to fix it>`. Error: {error} | Source Code: {code}"'
.\build-win\Debug\fusionc.exe tests\test_c\error.c c
```
- If the command fails or is unset, FusionC falls back to the built-in heuristic explainer.

Disable AI explicitly (fallback mode):

```powershell
$env:FUSIONC_DISABLE_AI='1'
.\build-win\Debug\fusionc.exe tests\test_c\sample.c c
Remove-Item Env:FUSIONC_DISABLE_AI
```

## Build (Windows)

Requires CMake and MSVC build tools.

```powershell
cmake -S . -B build-win
cmake --build build-win --config Debug
```

Result: `build-win\Debug\fusionc.exe`

## CLI Usage

```
fusionc <source-file> <language?> [--dump-tokens] [--dump-ast]
```

- `<language>` is optional; pass `c` or `custom` to override detection.
- `--dump-tokens` prints lexer tokens.
- `--dump-ast` prints AST.

Show help:

```powershell
.\build-win\Debug\fusionc.exe --help
```

## Run Examples

Run C sample with explicit language and diagnostics:

```powershell
.\build-win\Debug\fusionc.exe tests\test_c\sample.c c --dump-tokens --dump-ast
```

Run CustomLang sample:

```powershell
.\build-win\Debug\fusionc.exe tests\test_custom\sample.fsc custom --dump-tokens --dump-ast
```

Observe the printed exit code from the interpreter at the end.

## Development & Testing
- Reconfigure and rebuild after changes: `cmake -S . -B build-win` then `cmake --build build-win --config Debug`.
- Manual smoke test: run the C sample above and verify the reported exit code matches expectations (should be 30 for the provided sample arithmetic).
- AI diagnostics smoke test: run a file with an intentional syntax error and verify the output includes `Why:` and `Suggestion:` lines.
- Fallback smoke test: set `FUSIONC_DISABLE_AI=1` and verify the same syntax error appears in raw compiler form.
- Adjust or extend grammar/semantics in `frontend/` and update TAC builder/optimizer/backend accordingly.

## Limitations / Next Steps

- Only arithmetic expressions, declarations, assignments, and simple loops (`while`, `for`); no conditionals or complex control flow yet.
- Backend is an interpreter, not native codegen; can be replaced with real code generation later.
- Type system is minimal (int-focused) and does not yet validate function params.

## Troubleshooting

- `cmake` not found: reinstall CMake and add to PATH.
- Build fails for missing MSVC: install Visual Studio Build Tools (C++ workload).
- Unexpected language detection: pass the `<language>` hint (`c` or `custom`).
- Need raw compiler diagnostics only: set environment variable `FUSIONC_DISABLE_AI=1`.

