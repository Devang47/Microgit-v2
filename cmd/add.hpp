#pragma once

#include "root.hpp"
#include <string>
#include <vector>

namespace cmd
{
  extern Command *addCmd;

  // Updates index with path -> hash mapping
  bool UpdateIndex(const std::string &filePath, const std::string &hash);

  // Stage a specific file
  bool StageFile(const std::string &path, const std::string &fileName);

  // Add files to the repository's staging area
  int Add(const std::vector<std::string> &args);

  void InitAddCommand();
}
