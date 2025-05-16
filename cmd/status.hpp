#pragma once

#include "root.hpp"
#include <string>
#include <map>
#include <vector>

namespace cmd
{
  extern Command *statusCmd;

  // Get files in the working directory with their content hashes
  std::map<std::string, std::string> GetWorkingFiles();

  // Get files from the latest commit
  std::map<std::string, std::string> GetCommittedFiles();

  void InitStatusCommand();

  // Show status of working directory
  int Status(const std::vector<std::string> &args);
}
