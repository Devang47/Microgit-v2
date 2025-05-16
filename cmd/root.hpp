#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>

namespace cmd
{

  class Command
  {
  public:
    Command(const std::string &name, const std::string &shortDesc, const std::string &longDesc);
    virtual ~Command() = default;

    void AddCommand(Command *cmd);
    void SetRunFunc(std::function<void(const std::vector<std::string> &)> runFunc);
    void Execute(const std::vector<std::string> &args);

    std::string GetName() const { return name; }
    std::string GetShortDescription() const { return shortDescription; }
    std::string GetLongDescription() const { return longDescription; }

  private:
    std::string name;
    std::string shortDescription;
    std::string longDescription;
    std::function<void(const std::vector<std::string> &)> runFunction;
    std::map<std::string, Command *> subcommands;
  };

  extern Command *rootCmd;

  void InitCommands();

  // Show help information
  void ShowHelp();

  // Execute the specified command
  int Execute(int argc, char **argv);

} // namespace cmd
