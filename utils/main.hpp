#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <filesystem>

namespace utils
{
  // Default path for the repository
  const std::string DEFAULT_PATH = ".microgit";

  // SavePoint structure to store commit information
  struct SavePoint
  {
    std::string message;
    std::string timestamp;
    std::string parent;
    std::map<std::string, std::string> files; // filename -> hash
  };

  // Hash the content using SHA-256
  std::string HashContent(const std::vector<uint8_t> &content);

  // Write object to the repository
  bool WriteObject(const std::string &hash, const std::vector<uint8_t> &content);

  // Check if a file exists
  inline bool FileExists(const std::string &path)
  {
    return std::filesystem::exists(path);
  }

  // Read a file into a string
  inline std::string ReadFile(const std::string &path)
  {
    if (!FileExists(path))
    {
      return "";
    }
    std::ifstream file(path);
    if (!file.is_open())
    {
      return "";
    }
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>());
  }

  // Check if a string starts with a specific prefix
  inline bool starts_with(const std::string &str, const std::string &prefix)
  {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
  }

} // namespace utils
