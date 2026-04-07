#pragma once

#include <string>
#include <vector>

#include "ai_client.h"

namespace fusionc::ai
{

  class ErrorHandler
  {
  public:
    explicit ErrorHandler(AIClient client = AIClient());

    std::vector<std::string> buildUserFriendlyErrors(const std::vector<std::string> &compilerErrors, const std::string &sourceCode) const;

  private:
    AIClient client_;

    static bool isSyntaxDiagnostic(const std::string &error);
  };

} // namespace fusionc::ai
