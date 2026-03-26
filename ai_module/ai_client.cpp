#include "ai_client.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <sstream>

namespace fusionc::ai
{

  AIClient::AIClient()
  {
    // Set FUSIONC_DISABLE_AI=1 to test fallback behavior.
    const char *disable = std::getenv("FUSIONC_DISABLE_AI");
    enabled_ = !(disable != nullptr && std::string(disable) == "1");

    const char *cmdTmpl = std::getenv("FUSIONC_LLM_CMD_TEMPLATE");
    if (cmdTmpl != nullptr && std::string(cmdTmpl).size() > 0)
    {
      backend_ = AIBackend::Command;
      cmdTemplate_ = cmdTmpl;
    }
  }

  bool AIClient::isAvailable() const
  {
    return enabled_;
  }

  std::optional<ErrorInsight> AIClient::analyzeSyntaxError(const std::string &compilerError, const std::string &sourceCode) const
  {
    if (!enabled_)
    {
      return std::nullopt;
    }

    if (backend_ == AIBackend::Command)
    {
      if (auto llm = analyzeWithCommand(compilerError, sourceCode))
      {
        return llm;
      }
      // If command fails, fall back to heuristic below.
    }

    const std::string message = toLower(compilerError);

    if (message.find("expected ';'") != std::string::npos)
    {
      return ErrorInsight{
          "The statement is incomplete because the parser reached the end of a statement without a semicolon.",
          "Add ';' at the end of the statement, usually after an expression, declaration, or function call."};
    }

    if (message.find("expected '") != std::string::npos && message.find(" after ") != std::string::npos)
    {
      return ErrorInsight{
          "A required delimiter or keyword is missing, so the parser cannot continue with the expected grammar rule.",
          "Insert the missing token shown in the error near the location mentioned after 'after ...', then re-check surrounding braces and parentheses."};
    }

    if (message.find("unexpected token") != std::string::npos)
    {
      return ErrorInsight{
          "The parser found a token that does not fit this position in the language grammar.",
          "Check for a typo, missing operator, or missing separator before this token; fixing the previous line often resolves it."};
    }

    if (message.find("unknown token") != std::string::npos)
    {
      return ErrorInsight{
          "The lexer produced an unknown token, which usually means an invalid character sequence or unsupported symbol.",
          "Verify spelling of identifiers and operators, and remove unsupported characters around the reported line and column."};
    }

    if (message.find("expected expression") != std::string::npos)
    {
      return ErrorInsight{
          "An expression was required but missing, so the parser could not build a valid AST node.",
          "Provide a valid expression (identifier, literal, or operation) at the reported position."};
    }

    if (message.find("expected function") != std::string::npos)
    {
      return ErrorInsight{
          "The parser expected a valid function declaration but encountered a different token pattern.",
          "Ensure function syntax is complete: return type or 'fn', function name, parameter list in parentheses, and a body in braces."};
    }

    return ErrorInsight{
        "The compiler detected a syntax issue and could not match this code to a valid grammar pattern.",
        "Review nearby lines for missing punctuation, mismatched delimiters, or incomplete expressions."};
  }

  std::optional<ErrorInsight> AIClient::analyzeWithCommand(const std::string &compilerError, const std::string &sourceCode) const
  {
    if (cmdTemplate_.empty())
    {
      return std::nullopt;
    }

    const std::string command = renderCommand(cmdTemplate_, compilerError, sourceCode);
    const std::string output = runCommand(command);
    if (output.empty())
    {
      return std::nullopt;
    }

    // Command backend: return the full model output as the explanation and leave suggestion empty
    // so the caller can render only the LLM text.
    return ErrorInsight{output, ""};
  }

  std::string AIClient::escapeForCommandLine(const std::string& input)
  {
    std::string result;
    for (char c : input)
    {
      if (c == '"') result += "\\\"";
      else if (c == '\\') result += "\\\\";
      else if (c == '\n') result += " ";
      else if (c == '\r') result += " ";
      else result += c;
    }
    return result;
  }

  std::string AIClient::renderCommand(const std::string &tmpl, const std::string &errorText, const std::string &sourceCode)
  {
    std::string escapedErrorText = escapeForCommandLine(errorText);
    std::string escapedSourceCode = escapeForCommandLine(sourceCode);

    std::string cmd = tmpl;
    const std::string placeholder = "{error}";
    const std::string codePlaceholder = "{code}";
    std::size_t pos = 0;
    while ((pos = cmd.find(placeholder, pos)) != std::string::npos)
    {
      cmd.replace(pos, placeholder.size(), escapedErrorText);
      pos += escapedErrorText.size();
    }

    pos = 0;
    while ((pos = cmd.find(codePlaceholder, pos)) != std::string::npos)
    {
      cmd.replace(pos, codePlaceholder.size(), escapedSourceCode);
      pos += escapedSourceCode.size();
    }
    return cmd;
  }

  std::string AIClient::runCommand(const std::string &command)
  {
    std::string result;

#if defined(_WIN32)
    FILE *pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
      return result;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
      result.append(buffer);
      if (result.size() > 4000)
      {
        break;
      }
    }
    _pclose(pipe);
#else
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
      return result;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
      result.append(buffer);
      if (result.size() > 4000)
      {
        break;
      }
    }
    pclose(pipe);
#endif

    return result;
  }

  std::string AIClient::toLower(std::string text)
  {
    std::transform(text.begin(), text.end(), text.begin(),
                   [](unsigned char c)
                   { return static_cast<char>(std::tolower(c)); });
    return text;
  }

} // namespace fusionc::ai
