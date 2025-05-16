#pragma once

#include "root.hpp"
#include "../utils/main.hpp"
#include <string>
#include <map>
#include <vector>

namespace cmd
{
  extern Command *saveCmd;

  // Get the current HEAD commit hash
  std::string GetHead();

  // Set the HEAD and LATEST to point to the given commit hash
  bool SetHead(const std::string &hash);

  // Read the index file into a map
  std::map<std::string, std::string> ReadIndex();

  // Write a SavePoint to a new object and return its hash
  std::string WriteSavePointObject(const utils::SavePoint &savePoint);

  void InitSaveCommand();

  // Save staged changes to the repository (commit)
  int Save(const std::vector<std::string> &args);
}
