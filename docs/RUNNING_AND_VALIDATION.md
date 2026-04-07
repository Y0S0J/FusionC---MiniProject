# FusionC Runbook: Build, Run, Validate, and Understand Commands

This is a practical command guide for your current mini compiler.

## 1) What Type of Code Can This Compiler Run Right Now?

### Supported C-like subset (working)
- One function with no parameters.
- Declarations: `int x = ...;`
- Assignments: `x = ...;`
- Arithmetic: `+`, `-`, `*`, `/`
- Return statement: `return expr;`
- Braces and semicolon-based block syntax.

Example that works:

```c
int main() {
  int x = 10;
  int y = x + 5;
  y = y * 2;
  return y;
}
```

### Supported custom subset (current behavior)
- Similar statement model to above.
- Supports `fn main() { ... }` in current implementation.
- Supports `let` declarations and `print` statements in provided custom tests.

Working custom examples in current implementation:

```text
fn main() {
  let y = 20;
  print y;
  return 0;
}
```

## 2) Build Commands (Windows)

Run from project root.

### Configure

```powershell
cmake -S . -B build-win
```

What it does:
- Reads `CMakeLists.txt`
- Creates build files in `build-win/` for your toolchain

### Build

```powershell
cmake --build build-win --config Debug
```

What it does:
- Compiles C++ files
- Produces executable at `build-win/Debug/fusionc.exe`

## 3) Run Commands and Meaning

### Show help

```powershell
.\build-win\Debug\fusionc.exe --help
```

What it does:
- Prints CLI usage and options.

### Run C sample with stage dumps

```powershell
.\build-win\Debug\fusionc.exe tests\test_c\sample.c c --dump-tokens --dump-ast
```

What each argument means:
- `tests\test_c\sample.c`: input source file
- `c`: language hint (override detection)
- `--dump-tokens`: print lexer output
- `--dump-ast`: print parser AST

What success looks like:
- No errors section
- "Compilation pipeline completed successfully."
- "Program exit code: 30"

### Run custom sample from repo

```powershell
.\build-win\Debug\fusionc.exe tests\test_custom\sample.fsc custom --dump-tokens --dump-ast
```

Current expected output:
- Compilation pipeline success.
- Program output `30`.
- Program exit code `0`.

### Validate AI-assisted syntax diagnostics (new)

Create a small file with a syntax error:

```powershell
$tmp = Join-Path $env:TEMP 'fusionc_ai_error_demo.c'
@'
int main() {
  int x = 1
  return x;
}
'@ | Set-Content -Path $tmp -Encoding ascii
```

Run normally (AI enabled by default):

```powershell
.\build-win\Debug\fusionc.exe $tmp c
```

Expected behavior:
- Error output should include `Syntax error: ...` plus `Why:` and `Suggestion:` lines.

Validate fallback mode (AI disabled):

```powershell
$env:FUSIONC_DISABLE_AI='1'
.\build-win\Debug\fusionc.exe $tmp c
Remove-Item Env:FUSIONC_DISABLE_AI
```

Expected behavior:
- Error output should fall back to the original compiler message without AI explanation fields.

### Run custom print samples from repo

```powershell
.\build-win\Debug\fusionc.exe tests\test_custom\print.fsc custom --dump-tokens --dump-ast
.\build-win\Debug\fusionc.exe tests\test_custom\print_var.fsc custom --dump-tokens --dump-ast
```

What it does:
- Validates integer and string print paths in CustomLang.
- Confirms the current bundled custom tests execute successfully.

## 4) How to Check if Compiler is Working Correctly

Use this quick checklist:

1. Build succeeds with no compiler errors.
2. C sample command runs and returns exit code `30`.
3. Tokens include expected sequence (`int`, `main`, `{`, statements, `return`, `}`).
4. AST structure includes:
   - `Program`
   - `Function(main:int)`
   - `Declaration`, `Assignment`, `Return`
5. No semantic errors for valid C sample.
6. AI syntax diagnostics appear for malformed input (`Why` and `Suggestion` present).
7. Fallback mode (`FUSIONC_DISABLE_AI=1`) returns raw compiler diagnostics.
8. Custom samples (`sample.fsc`, `print.fsc`, `print_var.fsc`) run successfully with both explicit `custom` hint and auto-detection.

Optional negative tests:
- Use undeclared variable and verify semantic error appears.
- Remove semicolon and verify parser error appears.
- Use unknown symbol and verify lexer/parsing error behavior.

## 5) Do You Need to Do More to Run "Any" Code?

Yes. To run broader real-world code, you must extend the compiler.

For now it handles only a small educational subset. To support "any code" you would need:
1. Much larger grammar coverage.
2. Better type system.
3. Control-flow lowering in IR.
4. Function parameters and calls.
5. Real backend code generation and linking.

## 6) Recommended Next Validation Commands (After Every Change)

```powershell
cmake --build build-win --config Debug
.\build-win\Debug\fusionc.exe tests\test_c\sample.c c --dump-tokens --dump-ast
```

If both pass and exit code remains stable, your change likely did not break the main path.

If you touched diagnostics/AI behavior, also run:

```powershell
.\build-win\Debug\fusionc.exe $tmp c
$env:FUSIONC_DISABLE_AI='1'; .\build-win\Debug\fusionc.exe $tmp c; Remove-Item Env:FUSIONC_DISABLE_AI
```

## 7) Common CLI Patterns You Can Reuse

```powershell
# Auto-detect language by extension
.\build-win\Debug\fusionc.exe path\to\file.c --dump-tokens --dump-ast

# Force C
.\build-win\Debug\fusionc.exe path\to\file.any c --dump-tokens --dump-ast

# Force custom
.\build-win\Debug\fusionc.exe path\to\file.any custom --dump-tokens --dump-ast
```

Use language hints when detection is ambiguous.
