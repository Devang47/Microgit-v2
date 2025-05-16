#include "add.hpp"
#include "../utils/main.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace cmd
{
  Command *addCmd = nullptr;

  bool UpdateIndex(const std::string &filePath, const std::string &hash)
  {
    std::string indexPath = utils::DEFAULT_PATH + "/index";
    std::string existing;

    // Read existing index
    std::ifstream indexFile(indexPath);
    if (indexFile.is_open())
    {
      std::stringstream buffer;
      buffer << indexFile.rdbuf();
      existing = buffer.str();
      indexFile.close();
    }

    // Split into lines
    std::vector<std::string> lines;
    std::istringstream stream(existing);
    std::string line;
    while (std::getline(stream, line))
    {
      if (!line.empty())
      {
        lines.push_back(line);
      }
    }

    // Update or add entry
    bool found = false;
    for (size_t i = 0; i < lines.size(); i++)
    {
      if (lines[i].substr(0, filePath.length()) == filePath &&
          (lines[i].length() == filePath.length() || lines[i][filePath.length()] == ' '))
      {
        lines[i] = filePath + " " + hash;
        found = true;
        break;
      }
    }

    if (!found)
    {
      lines.push_back(filePath + " " + hash);
    }

    // Write back to file
    std::ofstream outFile(indexPath);
    if (!outFile.is_open())
    {
      return false;
    }

    for (size_t i = 0; i < lines.size(); i++)
    {
      outFile << lines[i];
      if (i < lines.size() - 1)
      {
        outFile << std::endl;
      }
    }

    outFile.close();
    return true;
  }

  bool StageFile(const std::string &path, const std::string &fileName)
  {
    try
    {
      // Read file content
      std::ifstream file(path, std::ios::binary);
      if (!file.is_open())
      {
        std::cerr << "Error reading file '" << fileName << "'" << std::endl;
        return false;
      }

      // Read file into vector
      std::vector<uint8_t> content(
          (std::istreambuf_iterator<char>(file)),
          std::istreambuf_iterator<char>());
      file.close();

      // Calculate hash
      std::string hash = utils::HashContent(content);

      // Write object
      if (!utils::WriteObject(hash, content))
      {
        std::cerr << "Error writing object for '" << fileName << "'" << std::endl;
        return false;
      }

      // Update index
      if (!UpdateIndex(path, hash))
      {
        std::cerr << "Error updating index for '" << fileName << "'" << std::endl;
        return false;
      }

      std::cout << "Added " << path << " (hash: " << hash << ")" << std::endl;
      return true;
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error: " << e.what() << std::endl;
      return false;
    }
  }

  int Add(const std::vector<std::string> &args)
  {
    // Check for the .microgit directory
    if (!fs::exists(utils::DEFAULT_PATH))
    {
      std::cerr << "Error: Not a MicroGit repository (or any parent up to mount point /)" << std::endl;
      return 1;
    }

    // Check if we have any arguments
    if (args.empty())
    {
      std::cerr << "Error: No files specified to add" << std::endl;
      std::cerr << "Usage: microgit add <file1> [file2] [file3] ..." << std::endl;
      return 1;
    }

    // Create the staging directory if it doesn't exist
    fs::path stagingDir = fs::path(utils::DEFAULT_PATH) / "staging";
    if (!fs::exists(stagingDir))
    {
      fs::create_directories(stagingDir);
    }

    int filesAdded = 0;
    int filesSkipped = 0;

    // Process each file
    for (const auto &file : args)
    {
      fs::path filePath = fs::path(file);

      // Check if the file exists
      if (!fs::exists(filePath))
      {
        std::cerr << "Warning: '" << file << "' did not match any files" << std::endl;
        filesSkipped++;
        continue;
      }

      // Read the file contents
      try
      {
        std::ifstream input(filePath, std::ios::binary);
        if (!input)
        {
          std::cerr << "Error: Cannot read file '" << file << "'" << std::endl;
          filesSkipped++;
          continue;
        }

        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(input), {});

        // Hash the content
        std::string hash = utils::HashContent(buffer);

        // Store the content in the objects directory
        if (!utils::WriteObject(hash, buffer))
        {
          std::cerr << "Error: Could not write to object store for '" << file << "'" << std::endl;
          filesSkipped++;
          continue;
        }

        // Create an entry in the staging area
        fs::path stagingPath = stagingDir / filePath.filename().string();
        std::ofstream stageFile(stagingPath);
        if (!stageFile)
        {
          std::cerr << "Error: Could not write to staging area for '" << file << "'" << std::endl;
          filesSkipped++;
          continue;
        }

        stageFile << hash << std::endl;
        stageFile.close();

        std::cout << "Added '" << file << "'" << std::endl;
        filesAdded++;
      }
      catch (const std::exception &e)
      {
        std::cerr << "Error processing '" << file << "': " << e.what() << std::endl;
        filesSkipped++;
      }
    }

    std::cout << "Summary: " << filesAdded << " file(s) added, " << filesSkipped << " file(s) skipped" << std::endl;
    return filesSkipped > 0 ? 1 : 0;
  }

  void InitAddCommand()
  {
    addCmd = new Command(
        "add",
        "Add files to the staging area",
        "Add files to the staging area for the next commit.\n\n"
        "Usage:\n"
        "  microgit add <file1> [file2 ...]  - Stage specific files\n"
        "  microgit add .                    - Stage all files in current directory\n\n"
        "The add command will:\n"
        "1. Calculate a SHA-256 hash of the file content\n"
        "2. Store the file content in the objects directory\n"
        "3. Update the index with the file path and corresponding hash\n\n"
        "Files in the .microgit/ and .git/ directories are automatically ignored.");

    addCmd->SetRunFunc([](const std::vector<std::string> &args)
                       {
      if (args.empty()) {
        std::cout << "Error: No files specified" << std::endl;
        return;
      }
      
      if (args[0] == ".") {
        // Stage all files in current directory
        try {
          for (const auto& entry : fs::recursive_directory_iterator(".")) {
            std::string path = entry.path().string();
            
            // Skip directories and special paths
            if (fs::is_directory(entry) || 
                utils::starts_with(path, utils::DEFAULT_PATH) || 
                utils::starts_with(path, ".git/")) {
              continue;
            }
            
            StageFile(path, entry.path().filename().string());
          }
        } catch (const fs::filesystem_error& e) {
          std::cerr << "Error reading directory: " << e.what() << std::endl;
        }
        return;
      }
      
      // Stage specific files
      for (const auto& file : args) {
        StageFile(file, file);
      } });

    rootCmd->AddCommand(addCmd);
  }
}
