#pragma once

#include "root.hpp"
#include <string>
#include <vector>

namespace cmd
{
  extern Command *removeCmd;

  // Remove files from staging area
  int Remove(const std::vector<std::string> &args);

  // Initialize the remove command
  void InitRemoveCommand();
} // namespace cmd
