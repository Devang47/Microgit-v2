#include "./cmd/root.hpp"
#include "./cmd/init.hpp"
#include "./cmd/add.hpp"
#include "./cmd/save.hpp"
#include "./cmd/log.hpp"
#include "./cmd/status.hpp"
#include "./cmd/checkout.hpp"
#include "./cmd/remove.hpp"

int main(int argc, char **argv)
{
  // Initialize commands
  cmd::rootCmd = new cmd::Command("microgit", "MicroGit version control system", "MicroGit - a lightweight version control system");

  cmd::InitInitCommand();
  cmd::InitAddCommand();
  cmd::InitSaveCommand();
  cmd::InitLogCommand();
  cmd::InitStatusCommand();
  cmd::InitCheckoutCommand();
  cmd::InitRemoveCommand();

  int result = cmd::Execute(argc, argv);

  // Cleanup
  delete cmd::rootCmd;

  return result;
}
