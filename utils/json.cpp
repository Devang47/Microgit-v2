#include "json.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

namespace utils
{
  class JSONImpl
  {
  public:
    static std::string Stringify(const SavePoint &savePoint)
    {
      json j;
      j["message"] = savePoint.message;
      j["timestamp"] = savePoint.timestamp;
      j["parent"] = savePoint.parent;
      j["files"] = savePoint.files;

      return j.dump(2); // Pretty print with 2-space indentation
    }

    static SavePoint Parse(const std::string &jsonStr)
    {
      try
      {
        json j = json::parse(jsonStr);

        SavePoint savePoint;
        savePoint.message = j["message"];
        savePoint.timestamp = j["timestamp"];
        savePoint.parent = j["parent"];
        savePoint.files = j["files"].get<std::map<std::string, std::string>>();

        return savePoint;
      }
      catch (const std::exception &e)
      {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
      }
    }

    static SavePoint LoadFromFile(const std::string &filePath)
    {
      try
      {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
          throw std::runtime_error("Could not open file: " + filePath);
        }

        json j;
        file >> j;

        SavePoint savePoint;
        savePoint.message = j["message"];
        savePoint.timestamp = j["timestamp"];
        savePoint.parent = j["parent"];
        savePoint.files = j["files"].get<std::map<std::string, std::string>>();

        return savePoint;
      }
      catch (const std::exception &e)
      {
        throw std::runtime_error("Failed to load JSON from file: " + std::string(e.what()));
      }
    }
  };

  JSON::JSON() : impl(std::make_unique<JSONImpl>()) {}
  JSON::~JSON() = default;

  std::string JSON::Stringify(const SavePoint &savePoint)
  {
    return JSONImpl::Stringify(savePoint);
  }

  SavePoint JSON::Parse(const std::string &jsonStr)
  {
    return JSONImpl::Parse(jsonStr);
  }

  SavePoint JSON::LoadFromFile(const std::string &filePath)
  {
    return JSONImpl::LoadFromFile(filePath);
  }
}
