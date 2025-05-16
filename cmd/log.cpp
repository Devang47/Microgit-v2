#include "log.hpp"
#include "../utils/main.hpp"
#include "../utils/json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace cmd
{
  Command *logCmd = nullptr;

  utils::SavePoint ReadCommit(const std::string &hash)
  {
    if (hash.empty())
    {
      return utils::SavePoint();
    }

    std::string commitPath = utils::DEFAULT_PATH + "/objects/" + hash;
    if (!utils::FileExists(commitPath))
    {
      return utils::SavePoint();
    }

    std::string commitContent = utils::ReadFile(commitPath);
    try
    {
      // Replace the non-existent FromJson method with Parse
      return utils::JSON::Parse(commitContent);
    }
    catch (const std::exception &e)
    {
      std::cerr << "Error parsing commit: " << e.what() << std::endl;
      return utils::SavePoint();
    }
  }

  int Log(const std::vector<std::string> &args)
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
    else
    {
      std::cerr << "Error: HEAD reference not found" << std::endl;
      return 1;
    }

    if (currentHash.empty())
    {
      std::cerr << "No commits yet" << std::endl;
      return 0;
    }

    // Determine how many commits to show (default to all)
    int limit = -1; // -1 means no limit
    if (!args.empty())
    {
      try
      {
        limit = std::stoi(args[0]);
      }
      catch (const std::exception &)
      {
        std::cerr << "Warning: Invalid limit parameter, showing all commits" << std::endl;
      }
    }

    // Track how many commits we've displayed
    int commits_shown = 0;

    // Start from HEAD and follow parent chain
    std::string hash = currentHash;
    while (!hash.empty())
    {
      if (limit > 0 && commits_shown >= limit)
      {
        break;
      }

      // Read savepoint from objects
      try
      {
        fs::path objectPath = fs::path(utils::DEFAULT_PATH) / "objects" / hash;
        if (!fs::exists(objectPath))
        {
          std::cerr << "Warning: Missing object for commit " << hash << std::endl;
          break;
        }

        std::ifstream file(objectPath);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        utils::SavePoint savePoint = utils::JSON::Parse(content);

        // Display commit information
        std::cout << "Commit: " << hash.substr(0, 8) << "..." << std::endl;
        std::cout << "Date:   " << savePoint.timestamp << std::endl;
        std::cout << std::endl;
        std::cout << "    " << savePoint.message << std::endl;
        std::cout << std::endl;

        // Move to parent
        hash = savePoint.parent;
        commits_shown++;
      }
      catch (const std::exception &e)
      {
        std::cerr << "Error reading commit " << hash << ": " << e.what() << std::endl;
        break;
      }
    }

    if (commits_shown == 0)
    {
      std::cout << "No commit history found." << std::endl;
    }

    return 0;
  }

  void InitLogCommand()
  {
    logCmd = new Command(
        "log",
        "Show commit history",
        "Show the commit history with the most recent commits first.\n\n"
        "Usage:\n"
        "  microgit log         - Show all commits\n"
        "  microgit log <n>     - Show only the last n commits\n\n"
        "Each commit shows:\n"
        "- Commit hash (abbreviated)\n"
        "- Date and time\n"
        "- Commit message");

    logCmd->SetRunFunc([](const std::vector<std::string> &args)
                       { Log(args); });

    rootCmd->AddCommand(logCmd);
  }

} // namespace cmd
