#include "checkout.hpp"
#include "../utils/main.hpp"
#include "../utils/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <map>

namespace fs = std::filesystem;

namespace cmd
{
  Command *checkoutCmd = nullptr;

  int Checkout(const std::vector<std::string> &args)
  {
    // Check for the .microgit directory
    if (!fs::exists(utils::DEFAULT_PATH))
    {
      std::cerr << "Error: Not a MicroGit repository (or any parent up to mount point /)" << std::endl;
      return 1;
    }

    // Check if we have the required arguments
    if (args.empty())
    {
      std::cerr << "Error: Missing commit hash or file name" << std::endl;
      std::cerr << "Usage: microgit checkout <commit> [file]" << std::endl;
      std::cerr << "       microgit checkout <file>" << std::endl;
      return 1;
    }

    std::string commitHash = args[0];
    bool singleFileMode = false;
    std::string targetFile = "";

    if (args.size() > 1)
    {
      // If we have more than one argument, the second one is a file name
      targetFile = args[1];
      singleFileMode = true;
    }

    // If only one argument and it's not a valid hash, assume it's a file
    fs::path objectPath = fs::path(utils::DEFAULT_PATH) / "objects" / commitHash;
    if (args.size() == 1 && !fs::exists(objectPath))
    {
      // Try to find the head commit
      std::ifstream headFile(fs::path(utils::DEFAULT_PATH) / "HEAD");
      std::string headHash;
      if (!headFile || !(headFile >> headHash))
      {
        std::cerr << "Error: Could not determine current HEAD" << std::endl;
        return 1;
      }

      // Set the file to checkout
      targetFile = commitHash;
      commitHash = headHash;
      singleFileMode = true;
    }

    // Load the commit
    objectPath = fs::path(utils::DEFAULT_PATH) / "objects" / commitHash;
    if (!fs::exists(objectPath))
    {
      std::cerr << "Error: Commit " << commitHash << " not found" << std::endl;
      return 1;
    }

    try
    {
      std::ifstream file(objectPath);
      std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
      utils::SavePoint savePoint = utils::JSON::Parse(content);

      if (singleFileMode)
      {
        // Checkout a single file
        if (savePoint.files.find(targetFile) == savePoint.files.end())
        {
          std::cerr << "Error: File '" << targetFile << "' not found in commit " << commitHash.substr(0, 8) << std::endl;
          return 1;
        }

        std::string fileHash = savePoint.files[targetFile];
        fs::path fileObjectPath = fs::path(utils::DEFAULT_PATH) / "objects" / fileHash;
        if (!fs::exists(fileObjectPath))
        {
          std::cerr << "Error: Object for file '" << targetFile << "' not found" << std::endl;
          return 1;
        }

        // Read the file content
        std::ifstream objectFile(fileObjectPath, std::ios::binary);
        std::vector<uint8_t> fileContent(std::istreambuf_iterator<char>(objectFile), {});

        // Write to the working directory
        std::ofstream outputFile(targetFile, std::ios::binary);
        outputFile.write(reinterpret_cast<const char *>(fileContent.data()), fileContent.size());

        std::cout << "Restored '" << targetFile << "' from commit " << commitHash.substr(0, 8) << std::endl;
      }
      else
      {
        // Checkout entire commit
        int filesRestored = 0;

        for (const auto &[filename, fileHash] : savePoint.files)
        {
          fs::path fileObjectPath = fs::path(utils::DEFAULT_PATH) / "objects" / fileHash;
          if (!fs::exists(fileObjectPath))
          {
            std::cerr << "Warning: Object for file '" << filename << "' not found, skipping" << std::endl;
            continue;
          }

          // Read the file content
          std::ifstream objectFile(fileObjectPath, std::ios::binary);
          std::vector<uint8_t> fileContent(std::istreambuf_iterator<char>(objectFile), {});

          // Write to the working directory
          std::ofstream outputFile(filename, std::ios::binary);
          outputFile.write(reinterpret_cast<const char *>(fileContent.data()), fileContent.size());

          filesRestored++;
        }

        // Update HEAD pointer
        std::ofstream headFile(fs::path(utils::DEFAULT_PATH) / "HEAD");
        headFile << commitHash;

        std::cout << "Checked out commit " << commitHash.substr(0, 8) << ": " << savePoint.message << std::endl;
        std::cout << filesRestored << " files restored" << std::endl;
      }

      return 0;
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error: Failed to checkout: " << e.what() << std::endl;
      return 1;
    }
  }

  void InitCheckoutCommand()
  {
    checkoutCmd = new Command(
        "checkout",
        "Checkout files from a specific commit",
        "Restore files from a specific commit to the working directory.\n\n"
        "Usage:\n"
        "  microgit checkout <commit>          - Restore all files from commit\n"
        "  microgit checkout <commit> <file>   - Restore specific file from commit\n"
        "  microgit checkout <file>            - Restore file from most recent commit\n\n"
        "When checking out a commit, HEAD will be updated to point to that commit.");

    checkoutCmd->SetRunFunc([](const std::vector<std::string> &args)
                            { Checkout(args); });

    rootCmd->AddCommand(checkoutCmd);
  }

} // namespace cmd
