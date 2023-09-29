#pragma once

#include <IR/IRCommon.h>
#include <DataStructures/List.hpp>

#include <array>
#include <vector>

namespace koda {

class Instruction;
class Graph;

class BasicBlock : public IntrusiveList {
public:
  using BBVector = std::vector<BasicBlock *>;

  virtual ~BasicBlock() = default;

private:
  bbid_t m_id;

  BBVector m_predecessors;
  // Max 2 successors are possible, since ISA doesn't have select operator
  std::array<BasicBlock *, 2> m_successors;
  
  Graph *m_graph;
};

} // namespace koda