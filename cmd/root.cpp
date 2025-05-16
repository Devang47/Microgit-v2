#include "root.hpp"
#include "init.hpp"
#include "add.hpp"
#include "save.hpp"
#include "log.hpp"
#include "status.hpp"
#include "checkout.hpp"
#include "remove.hpp"
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <vector>

namespace cmd
{
  // Root command instance
  Command *rootCmd = nullptr;

  // Command class implementation
  Command::Command(const std::string &name, const std::string &shortDesc, const std::string &longDesc)
      : name(name), shortDescription(shortDesc), longDescription(longDesc) {}

  void Command::AddCommand(Command *cmd)
  {
    if (cmd)
    {
      subcommands[cmd->GetName()] = cmd;
    }
  }

  void Command::SetRunFunc(std::function<void(const std::vector<std::string> &)> runFunc)
  {
    runFunction = runFunc;
  }

  void Command::Execute(const std::vector<std::string> &args)
  {
    if (args.empty() || args[0] == "--help" || args[0] == "-h")
    {
      // Show help for this command
      std::cout << name << " - " << shortDescription << std::endl;
      std::cout << std::endl;
      std::cout << longDescription << std::endl;

      if (!subcommands.empty())
      {
        std::cout << std::endl
                  << "Subcommands:" << std::endl;
        for (const auto &[name, cmd] : subcommands)
        {
          std::cout << "  " << name << " - " << cmd->GetShortDescription() << std::endl;
        }
      }
      return;
    }

    // Check if first arg is a subcommand
    if (!subcommands.empty())
    {
      auto it = subcommands.find(args[0]);
      if (it != subcommands.end())
      {
        // Execute subcommand with remaining args
        std::vector<std::string> subArgs(args.begin() + 1, args.end());
        it->second->Execute(subArgs);
        return;
      }
    }

    // Execute this command's function if available
    if (runFunction)
    {
      runFunction(args);
    }
    else
    {
      std::cout << "Command " << name << " not implemented" << std::endl;
    }
  }

  // External declarations (not definitions)
  // These functions are defined in their respective .cpp files
  extern int Init(const std::vector<std::string> &args);
  extern int Add(const std::vector<std::string> &args);
  extern int Save(const std::vector<std::string> &args);
  extern int Log(const std::vector<std::string> &args);
  extern int Status(const std::vector<std::string> &args);
  extern int Checkout(const std::vector<std::string> &args);
  extern int Remove(const std::vector<std::string> &args);

  void ShowHelp()
  {
    std::cout << "MicroGit - Lightweight Version Control System\n";
    std::cout << "Usage: microgit <command> [arguments]\n\n";
    std::cout << "Available commands:\n";
    std::cout << "  init     - Initialize a new MicroGit repository\n";
    std::cout << "  add      - Add files to the staging area\n";
    std::cout << "  save     - Save staged changes to the repository\n";
    std::cout << "  log      - Show commit history log\n";
    std::cout << "  status   - Show working directory status\n";
    std::cout << "  checkout - Checkout files from a commit\n";
    std::cout << "  remove   - Remove files from staging area\n";
    std::cout << "  --help   - Show this help message\n";
    std::cout << "\nFor more information, use 'microgit <command> --help'\n";
  }

  int Execute(int argc, char **argv)
  {
    if (argc < 2)
    {
      ShowHelp();
      return 0;
    }

    std::string cmd = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; i++)
    {
      args.push_back(argv[i]);
    }

    if (cmd == "--help" || cmd == "-h")
    {
      ShowHelp();
      return 0;
    }
    else if (cmd == "init")
    {
      return Init(args);
    }
    else if (cmd == "add")
    {
      return Add(args);
    }
    else if (cmd == "save")
    {
      return Save(args);
    }
    else if (cmd == "log")
    {
      return Log(args);
    }
    else if (cmd == "status")
    {
      return Status(args);
    }
    else if (cmd == "checkout")
    {
      return Checkout(args);
    }
    else if (cmd == "remove")
    {
      return Remove(args);
    }
    else
    {
      std::cout << "Unknown command: " << cmd << std::endl;
      ShowHelp();
      return 1;
    }
  }

} // namespace cmd
