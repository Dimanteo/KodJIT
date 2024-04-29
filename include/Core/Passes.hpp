#pragma once

#include <DataStructures/Tree.hpp>
#include <IR/BasicBlock.hpp>

namespace koda {

class Compiler;

// Base class for all passes
struct PassI {
  virtual ~PassI() = default;
  virtual void run(Compiler &compiler) = 0;
};

} // namespace koda