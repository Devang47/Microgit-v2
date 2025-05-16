#include "status.hpp"
#include "../utils/main.hpp"
#include "../utils/json.hpp"
#include "save.hpp" // Add this include for GetHead
#include "log.hpp"  // Add this include for ReadCommit
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>
#include <set>

namespace fs = std::filesystem;

namespace cmd
{
  Command *statusCmd = nullptr;

  std::map<std::string, std::string> GetWorkingFiles()
  {
    std::map<std::string, std::string> files;

    try
    {
      for (const auto &entry : fs::directory_iterator("."))
      {
        if (entry.path() == utils::DEFAULT_PATH ||
            entry.path() == ".git" ||
            entry.is_directory())
        {
          continue;
        }

        std::ifstream file(entry.path(), std::ios::binary);
        std::vector<uint8_t> content((std::istreambuf_iterator<char>(file)), {});
        std::string hash = utils::HashContent(content);
        files[entry.path().string()] = hash;
      }
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error reading working directory: " << e.what() << std::endl;
    }

    return files;
  }

  std::map<std::string, std::string> GetCommittedFiles()
  {
    std::map<std::string, std::string> files;
    std::string head = cmd::GetHead(); // Assuming GetHead() is defined in save.hpp

    if (head.empty())
    {
      return files;
    }

    try
    {
      utils::SavePoint savePoint = ReadCommit(head); // Using ReadCommit from log.hpp
      files = savePoint.files;
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error reading committed files: " << e.what() << std::endl;
    }

    return files;
  }

  int Status(const std::vector<std::string> &args)
  {
    // Check for the .microgit directory
    if (!fs::exists(utils::DEFAULT_PATH))
    {
      std::cerr << "Error: Not a MicroGit repository (or any parent up to mount point /)" << std::endl;
      return 1;
    }

    // Get current HEAD
    std::string currentHash = "";
    fs::path headFile = fs::path(utils::DEFAULT_PATH) / "HEAD";
    if (fs::exists(headFile))
    {
      std::ifstream head(headFile);
      if (head)
      {
        std::getline(head, currentHash);
      }
    }

    // Track files from HEAD, staged files, and working directory
    std::map<std::string, std::string> headFiles;   // filename -> hash
    std::map<std::string, std::string> stagedFiles; // filename -> hash
    std::set<std::string> workingDirFiles;          // just filenames

    // Get files from HEAD
    if (!currentHash.empty())
    {
      try
      {
        fs::path objectPath = fs::path(utils::DEFAULT_PATH) / "objects" / currentHash;
        if (fs::exists(objectPath))
        {
          std::ifstream file(objectPath);
          std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
          utils::SavePoint savePoint = utils::JSON::Parse(content);
          headFiles = savePoint.files;
        }
      }
      catch (const std::exception &e)
      {
        std::cerr << "Warning: Could not read HEAD commit: " << e.what() << std::endl;
      }
    }

    // Get staged files
    fs::path stagingDir = fs::path(utils::DEFAULT_PATH) / "staging";
    if (fs::exists(stagingDir))
    {
      for (const auto &entry : fs::directory_iterator(stagingDir))
      {
        std::string filename = entry.path().filename().string();
        std::ifstream stageFile(entry.path());
        if (stageFile)
        {
          std::string hash;
          std::getline(stageFile, hash);
          stagedFiles[filename] = hash;
        }
      }
    }

    // Get files in working directory (excluding .microgit)
    for (const auto &entry : fs::directory_iterator("."))
    {
      if (entry.path() != utils::DEFAULT_PATH && !entry.is_directory())
      {
        workingDirFiles.insert(entry.path().filename().string());
      }
    }

    // Display branch information
    std::cout << "On branch main" << std::endl;
    if (currentHash.empty())
    {
      std::cout << "No commits yet" << std::endl;
    }
    else
    {
      std::cout << "HEAD: " << currentHash.substr(0, 8) << std::endl;
    }
    std::cout << std::endl;

    // Display staged changes
    if (!stagedFiles.empty())
    {
      std::cout << "Changes to be committed:" << std::endl;
      std::cout << "  (use \"microgit remove <file>...\" to unstage)" << std::endl;
      std::cout << std::endl;

      for (const auto &[file, hash] : stagedFiles)
      {
        // Check if file is new or modified
        if (headFiles.find(file) == headFiles.end())
        {
          std::cout << "        new file:   " << file << std::endl;
        }
        else if (headFiles[file] != hash)
        {
          std::cout << "        modified:   " << file << std::endl;
        }
      }
      std::cout << std::endl;
    }

    // Display working directory changes (files that are tracked but not staged)
    std::vector<std::string> modifiedFiles;
    std::vector<std::string> untrackedFiles;

    for (const auto &file : workingDirFiles)
    {
      // Skip files that are already staged
      if (stagedFiles.find(file) != stagedFiles.end())
      {
        continue;
      }

      // If file is tracked (in HEAD) but modified
      if (headFiles.find(file) != headFiles.end())
      {
        // Read current content and compute hash
        try
        {
          std::ifstream input(file, std::ios::binary);
          if (input)
          {
            std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});
            std::string hash = utils::HashContent(buffer);

            if (hash != headFiles[file])
            {
              modifiedFiles.push_back(file);
            }
          }
        }
        catch (const std::exception &)
        {
          // If we can't read the file, consider it modified
          modifiedFiles.push_back(file);
        }
      }
      else
      {
        // File is not in HEAD, so it's untracked
        untrackedFiles.push_back(file);
      }
    }

    if (!modifiedFiles.empty())
    {
      std::cout << "Changes not staged for commit:" << std::endl;
      std::cout << "  (use \"microgit add <file>...\" to update what will be committed)" << std::endl;
      std::cout << std::endl;

      for (const auto &file : modifiedFiles)
      {
        std::cout << "        modified:   " << file << std::endl;
      }
      std::cout << std::endl;
    }

    if (!untrackedFiles.empty())
    {
      std::cout << "Untracked files:" << std::endl;
      std::cout << "  (use \"microgit add <file>...\" to include in what will be committed)" << std::endl;
      std::cout << std::endl;

      for (const auto &file : untrackedFiles)
      {
        std::cout << "        " << file << std::endl;
      }
      std::cout << std::endl;
    }

    if (stagedFiles.empty() && modifiedFiles.empty() && untrackedFiles.empty())
    {
      std::cout << "Nothing to commit, working tree clean" << std::endl;
    }

    return 0;
  }

  void InitStatusCommand()
  {
    statusCmd = new Command(
        "status",
        "Show working tree status",
        "Display the state of the working directory and the staging area.\n"
        "Shows which changes have been staged, which haven't, and which files\n"
        "aren't being tracked by MicroGit.");

    statusCmd->SetRunFunc([](const std::vector<std::string> &args)
                          { Status(args); });

    rootCmd->AddCommand(statusCmd);
  }

} // namespace cmd
