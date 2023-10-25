#include <IR/BasicBlock.hpp>

#include <algorithm>

namespace koda {

void BasicBlock::setCondSuccessors(BasicBlock *false_bb, BasicBlock *true_bb) {
  setFalseSuccessor(false_bb);
  setTrueSuccessor(true_bb);
}

bool BasicBlock::hasSuccessor() const {
  return std::any_of(m_successors.cbegin(), m_successors.cend(),
                     [](const BasicBlock *bb) { return bb != nullptr; });
}

void BasicBlock::for_each_succ(std::function<void(BasicBlock &)> funct) const {
  for (BasicBlock *bb : m_successors) {
    if (bb != nullptr) {
      funct(*bb);
    }
  }
}

} // namespace koda