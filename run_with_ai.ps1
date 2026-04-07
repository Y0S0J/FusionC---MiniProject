param (
    [string]$File,
    [string]$Language
)

if (-not $File -or -not $Language) {
    Write-Host "Usage: .\run_with_ai.ps1 <file_path> <language_id>"
    Write-Host "Example: .\run_with_ai.ps1 tests\test_c\sample_error.c c"
    exit
}

# The prompt instructs Llama 3 to output exactly an Explanation and a Suggestion.
# It explicitly forbids empty lines and full-file rewrites to keep the terminal output clean.
$Prompt = 'ollama run llama3:8b-instruct-q4_K_M "You are an expert compiler assistant. Output EXACTLY two lines with no blank lines in between. Line 1: `Explanation: [concise reason for the error]`. Line 2: `Suggestion: [ONLY the corrected line of code, NOT the entire file]`. Absolutely no markdown, backticks, or other text. Error: {error} | Source Code: {code}"'

$env:FUSIONC_LLM_CMD_TEMPLATE = $Prompt

Write-Host "Running FusionC on $File with AI assistance..."
.\build-win\Debug\fusionc.exe $File $Language
