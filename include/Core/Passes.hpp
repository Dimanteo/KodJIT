#pragma once

#include <DataStructures/Tree.hpp>
#include <IR/BasicBlock.hpp>


namespace koda {

class ProgramGraph;

// Base class for all passes
struct PassI {
  virtual ~PassI() = default;
  virtual void run(ProgramGraph &graph) = 0;
};

} // namespace koda