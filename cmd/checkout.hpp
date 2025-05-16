#pragma once

#include "root.hpp"
#include <string>
#include <vector>

namespace cmd
{
  extern Command *checkoutCmd;

  // Checkout files or commits from the repository
  int Checkout(const std::vector<std::string> &args);

  // Initialize the checkout command
  void InitCheckoutCommand();
} // namespace cmd
