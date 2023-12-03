#include <IR/BasicBlock.hpp>

#include <algorithm>

namespace koda {

void BasicBlock::setCondSuccessors(BasicBlock *false_bb, BasicBlock *true_bb) {
  setSuccessor(TRUE_IDX, true_bb);
  setSuccessor(FALSE_IDX, false_bb);
}

bool BasicBlock::hasSuccessor() const {
  return !m_successors.empty();
}


} // namespace koda