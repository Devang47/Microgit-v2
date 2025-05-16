#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "main.hpp"

namespace utils
{
  // Forward declaration of our JSON implementation class
  class JSONImpl;

  // A simple JSON wrapper class for our needs
  class JSON
  {
  public:
    JSON();
    ~JSON();

    // Create JSON from SavePoint
    static std::string Stringify(const SavePoint &savePoint);

    // Parse JSON to SavePoint
    static SavePoint Parse(const std::string &jsonStr);

    // Load JSON from file and parse to SavePoint
    static SavePoint LoadFromFile(const std::string &filePath);

  private:
    std::unique_ptr<JSONImpl> impl;
  };
}
