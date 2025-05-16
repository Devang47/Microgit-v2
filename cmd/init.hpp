#pragma once

#include "root.hpp"
#include <vector>
#include <string>

namespace cmd
{
  extern Command *initCmd;

  void InitInitCommand();

  // Initialize a new MicroGit repository
  int Init(const std::vector<std::string> &args);
} // namespace cmd
