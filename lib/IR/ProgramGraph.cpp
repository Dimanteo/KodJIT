#include <IR/ProgramGraph.hpp>

namespace koda {
BasicBlock *ProgramGraph::createBasicBlock() {
  bbid_t id = m_bb_arena.size();
  auto new_bb = new BasicBlock(id, *this);
  m_bb_arena.emplace_back(new_bb);
  return m_bb_arena.back().get();
}
} // namespace koda
