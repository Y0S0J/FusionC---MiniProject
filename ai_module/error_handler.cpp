#include "error_handler.h"

#include <algorithm>
#include <cctype>

namespace fusionc::ai
{

  namespace
  {
    std::string toLower(std::string text)
    {
      std::transform(text.begin(), text.end(), text.begin(),
                     [](unsigned char c)
                     { return static_cast<char>(std::tolower(c)); });
      return text;
    }
  } // namespace

  ErrorHandler::ErrorHandler(AIClient client)
      : client_(std::move(client))
  {
  }

  std::vector<std::string> ErrorHandler::buildUserFriendlyErrors(const std::vector<std::string> &compilerErrors, const std::string &sourceCode) const
  {
    std::vector<std::string> transformed;
    transformed.reserve(compilerErrors.size());

    for (const auto &error : compilerErrors)
    {
      if (!isSyntaxDiagnostic(error))
      {
        transformed.push_back(error);
        continue;
      }

      const auto insight = client_.analyzeSyntaxError(error, sourceCode);
      if (!insight.has_value())
      {
        // AI unavailable: keep the original compiler diagnostics.
        transformed.push_back(error);
        continue;
      }

      if (insight->suggestion.empty())
      {
        // Command-based LLM: emit only the model output.
        transformed.push_back(insight->explanation);
      }
      else
      {
        transformed.push_back("Syntax error: " + error + "\n    Why: " + insight->explanation + "\n    Suggestion: " + insight->suggestion);
      }
    }

    return transformed;
  }

  bool ErrorHandler::isSyntaxDiagnostic(const std::string &error)
  {
    const std::string lower = toLower(error);
    return lower.find("expected") != std::string::npos ||
           lower.find("unexpected token") != std::string::npos ||
           lower.find("unknown token") != std::string::npos ||
           lower.find("syntax") != std::string::npos;
  }

} // namespace fusionc::ai
