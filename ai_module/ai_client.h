#pragma once

#include <optional>
#include <string>

namespace fusionc::ai
{

  enum class AIBackend
  {
    Heuristic,
    Command
  };

  struct ErrorInsight
  {
    std::string explanation;
    std::string suggestion;
  };

  class AIClient
  {
  public:
    AIClient();

    bool isAvailable() const;
    std::optional<ErrorInsight> analyzeSyntaxError(const std::string &compilerError, const std::string &sourceCode = "") const;

  private:
    bool enabled_ = true;
    AIBackend backend_ = AIBackend::Heuristic;
    std::string cmdTemplate_;

    std::optional<ErrorInsight> analyzeWithCommand(const std::string &compilerError, const std::string &sourceCode) const;
    static std::string renderCommand(const std::string &tmpl, const std::string &errorText, const std::string &sourceCode);
    static std::string escapeForCommandLine(const std::string& input);
    static std::string runCommand(const std::string &command);
    static std::string toLower(std::string text);
  };

} // namespace fusionc::ai
