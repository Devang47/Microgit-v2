#include "remove.hpp"
#include "root.hpp"
#include "../utils/main.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

namespace cmd
{
  Command *removeCmd = nullptr;

  int Remove(const std::vector<std::string> &args)
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
      std::cerr << "Error: No files specified to remove" << std::endl;
      std::cerr << "Usage: microgit remove <file1> [file2] [file3] ..." << std::endl;
      return 1;
    }

    fs::path stagingDir = fs::path(utils::DEFAULT_PATH) / "staging";
    if (!fs::exists(stagingDir))
    {
      std::cerr << "Error: No files staged for commit" << std::endl;
      return 1;
    }

    int filesRemoved = 0;
    int filesSkipped = 0;

    for (const auto &file : args)
    {
      fs::path stagePath = stagingDir / file;
      if (fs::exists(stagePath))
      {
        try
        {
          fs::remove(stagePath);
          std::cout << "Unstaged '" << file << "'" << std::endl;
          filesRemoved++;
        }
        catch (const std::exception &e)
        {
          std::cerr << "Error removing '" << file << "' from staging: " << e.what() << std::endl;
          filesSkipped++;
        }
      }
      else
      {
        std::cerr << "Warning: '" << file << "' not found in staging area" << std::endl;
        filesSkipped++;
      }
    }

    std::cout << "Summary: " << filesRemoved << " file(s) removed from staging, "
              << filesSkipped << " file(s) skipped" << std::endl;
    return filesSkipped > 0 ? 1 : 0;
  }

  void InitRemoveCommand()
  {
    removeCmd = new Command(
        "remove",
        "Remove files from the staging area",
        "Remove files from the staging area, effectively un-staging them.\n\n"
        "Usage:\n"
        "  microgit remove <file1> [file2 ...]  - Remove specific files from staging\n"
        "  microgit remove .                    - Remove all files from staging\n\n"
        "This command will:\n"
        "1. Remove the specified files from the index\n"
        "2. Keep the files in your working directory\n"
        "3. Allow you to re-stage them later if needed");

    removeCmd->SetRunFunc([](const std::vector<std::string> &args)
                          {
      if (args.empty()) {
        std::cout << "Error: No files specified" << std::endl;
        return;
      }
      
      if (args[0] == ".") {
        // Remove all from staging
        std::string indexPath = utils::DEFAULT_PATH + "/index";
        std::ofstream indexFile(indexPath, std::ios::trunc);
        indexFile.close();
        return;
      }
      
      // Remove specific files from staging
      std::string indexPath = utils::DEFAULT_PATH + "/index";
      
      try {
        // Read current index
        std::ifstream file(indexPath);
        if (!file.is_open()) {
          std::cout << "Failed to read index file" << std::endl;
          return;
        }
        
        std::string line;
        std::vector<std::string> lines;
        while (std::getline(file, line)) {
          if (!line.empty()) {
            bool keep = true;
            
            // Check if this line starts with any of the files to remove
            for (const auto& fileToRemove : args) {
              if (line.substr(0, fileToRemove.size()) == fileToRemove) {
                keep = false;
                break;
              }
            }
            
            if (keep) {
              lines.push_back(line);
            }
          }
        }
        file.close();
        
        // Write back filtered index
        std::ofstream outFile(indexPath);
        for (size_t i = 0; i < lines.size(); i++) {
          outFile << lines[i];
          if (i < lines.size() - 1) {
            outFile << std::endl;
          }
        }
        
      } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
      } });

    rootCmd->AddCommand(removeCmd);
  }
}
