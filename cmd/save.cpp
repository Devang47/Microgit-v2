#include "save.hpp"
#include "../utils/main.hpp"
#include "../utils/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <map>
#include <future>

namespace fs = std::filesystem;

namespace cmd
{
  Command *saveCmd = nullptr;

  std::string GetHead()
  {
    std::string headPath = utils::DEFAULT_PATH + "/HEAD";

    try
    {
      std::ifstream file(headPath);
      if (!file.is_open())
      {
        return "";
      }

      std::string head;
      std::getline(file, head);
      return head;
    }
    catch (const std::exception &e)
    {
      return "";
    }
  }

  bool SetHead(const std::string &hash)
  {
    std::string headPath = utils::DEFAULT_PATH + "/HEAD";
    std::string latestPath = utils::DEFAULT_PATH + "/LATEST";

    // Use futures to write both files concurrently
    auto writeHead = std::async(std::launch::async, [&]()
                                {
      std::ofstream file(headPath);
      if (!file.is_open()) {
        return false;
      }
      file << hash;
      return true; });

    auto writeLatest = std::async(std::launch::async, [&]()
                                  {
      std::ofstream file(latestPath);
      if (!file.is_open()) {
        return false;
      }
      file << hash;
      return true; });

    // Wait for both operations to complete
    bool headResult = writeHead.get();
    bool latestResult = writeLatest.get();

    return headResult && latestResult;
  }

  std::map<std::string, std::string> ReadIndex()
  {
    std::map<std::string, std::string> index;
    std::string indexPath = utils::DEFAULT_PATH + "/index";

    try
    {
      std::ifstream file(indexPath);
      if (!file.is_open())
      {
        return index;
      }

      std::string line;
      while (std::getline(file, line))
      {
        if (line.empty())
          continue;

        size_t spacePos = line.find_first_of(' ');
        if (spacePos != std::string::npos)
        {
          std::string path = line.substr(0, spacePos);
          std::string hash = line.substr(spacePos + 1);
          index[path] = hash;
        }
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error reading index: " << e.what() << std::endl;
    }

    return index;
  }

  std::string WriteSavePointObject(const utils::SavePoint &savePoint)
  {
    try
    {
      // Convert SavePoint to JSON
      std::string jsonData = utils::JSON::Stringify(savePoint);

      // Hash the JSON data
      std::vector<uint8_t> content(jsonData.begin(), jsonData.end());
      std::string hash = utils::HashContent(content);

      // Write the object
      if (!utils::WriteObject(hash, content))
      {
        throw std::runtime_error("Failed to write object");
      }

      return hash;
    }
    catch (const std::exception &e)
    {
      throw std::runtime_error("Failed to write SavePoint: " + std::string(e.what()));
    }
  }

  int Save(const std::vector<std::string> &args)
  {
    // Check for the .microgit directory
    if (!fs::exists(utils::DEFAULT_PATH))
    {
      std::cerr << "Error: Not a MicroGit repository (or any parent up to mount point /)" << std::endl;
      return 1;
    }

    // Check for staging directory
    fs::path stagingDir = fs::path(utils::DEFAULT_PATH) / "staging";
    if (!fs::exists(stagingDir) || fs::is_empty(stagingDir))
    {
      std::cerr << "Error: No changes staged for saving" << std::endl;
      std::cerr << "Use 'microgit add <file>' to stage files" << std::endl;
      return 1;
    }

    // Get commit message
    std::string message;
    if (args.empty())
    {
      std::cout << "Enter commit message: ";
      std::getline(std::cin, message);
      if (message.empty())
      {
        std::cerr << "Error: Empty commit message" << std::endl;
        return 1;
      }
    }
    else
    {
      message = args[0];
      // Join additional arguments as part of the message
      for (size_t i = 1; i < args.size(); i++)
      {
        message += " " + args[i];
      }
    }

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::string timestamp = std::ctime(&time_t_now);
    // Remove trailing newline from ctime
    if (!timestamp.empty() && timestamp[timestamp.length() - 1] == '\n')
    {
      timestamp.erase(timestamp.length() - 1);
    }

    // Get current HEAD
    std::string parent = "";
    fs::path headFile = fs::path(utils::DEFAULT_PATH) / "HEAD";
    if (fs::exists(headFile))
    {
      std::ifstream head(headFile);
      if (head)
      {
        std::getline(head, parent);
      }
    }

    // Create SavePoint with staged files
    utils::SavePoint savePoint;
    savePoint.message = message;
    savePoint.timestamp = timestamp;
    savePoint.parent = parent;

    // Process staging directory
    for (const auto &entry : fs::directory_iterator(stagingDir))
    {
      std::string filename = entry.path().filename().string();
      std::ifstream stageFile(entry.path());
      if (stageFile)
      {
        std::string hash;
        std::getline(stageFile, hash);
        savePoint.files[filename] = hash;
      }
    }

    // Convert SavePoint to JSON
    std::string jsonData = utils::JSON::Stringify(savePoint);

    // Hash the savepoint data
    std::vector<uint8_t> savePointData(jsonData.begin(), jsonData.end());
    std::string savePointHash = utils::HashContent(savePointData);

    // Write savepoint to objects
    if (!utils::WriteObject(savePointHash, savePointData))
    {
      std::cerr << "Error: Could not write savepoint to object store" << std::endl;
      return 1;
    }

    // Update HEAD
    std::ofstream head(headFile);
    if (!head)
    {
      std::cerr << "Error: Could not update HEAD reference" << std::endl;
      return 1;
    }
    head << savePointHash;
    head.close();

    // Clear staging directory
    for (const auto &entry : fs::directory_iterator(stagingDir))
    {
      fs::remove(entry.path());
    }

    std::cout << "Saved [" << savePointHash.substr(0, 8) << "]: " << message << std::endl;
    return 0;
  }

  void InitSaveCommand()
  {
    saveCmd = new Command(
        "save",
        "Save the current state of staged files",
        "Save the current state of all staged files as a new commit.\n"
        "This command requires a commit message that describes the changes being saved.\n"
        "The staged files will be committed and the staging area will be cleared after the save.");

    saveCmd->SetRunFunc([](const std::vector<std::string> &args)
                        { Save(args); });

    rootCmd->AddCommand(saveCmd);
  }
}
