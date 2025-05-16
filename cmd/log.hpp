#pragma once

#include <string>
#include <vector>
#include "root.hpp"
#include "../utils/main.hpp"

namespace cmd
{
  extern Command *logCmd;

  // Read a commit from its hash
  utils::SavePoint ReadCommit(const std::string &hash);

  void InitLogCommand();

  // Show commit history log
  int Log(const std::vector<std::string> &args);
}
