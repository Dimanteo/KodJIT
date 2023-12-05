#include <IR/BasicBlock.hpp>

#include <algorithm>

namespace koda {

void BasicBlock::set_cond_successors(BasicBlock *false_bb, BasicBlock *true_bb) {
  set_successor(TRUE_IDX, true_bb);
  set_successor(FALSE_IDX, false_bb);
}

bool BasicBlock::has_successor() const { return !m_successors.empty(); }

} // namespace koda