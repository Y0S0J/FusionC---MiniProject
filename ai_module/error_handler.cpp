#include "error_handler.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

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

    std::string extractSuggestion(const std::string& explanation)
    {
      const std::string kw = "Suggestion:";
      size_t pos = explanation.find(kw);
      if (pos == std::string::npos) return "";

      std::string suggestion = explanation.substr(pos + kw.length());
      size_t first = suggestion.find_first_not_of(" \t\r\n");
      if (std::string::npos == first) return "";
      size_t last = suggestion.find_last_not_of(" \t\r\n");
      return suggestion.substr(first, (last - first + 1));
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
    std::unordered_set<std::string> seenSuggestions;

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
        std::string parsedSuggestion = extractSuggestion(insight->explanation);
        if (!parsedSuggestion.empty())
        {
          if (seenSuggestions.find(parsedSuggestion) != seenSuggestions.end()) continue;
          seenSuggestions.insert(parsedSuggestion);
        }
        else
        {
          if (seenSuggestions.find(insight->explanation) != seenSuggestions.end()) continue;
          seenSuggestions.insert(insight->explanation);
        }
        transformed.push_back(insight->explanation);
      }
      else
      {
        if (seenSuggestions.find(insight->suggestion) != seenSuggestions.end()) continue;
        seenSuggestions.insert(insight->suggestion);
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
