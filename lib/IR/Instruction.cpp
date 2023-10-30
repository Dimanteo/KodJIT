#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>
#include <IR/IRBuilder.hpp>

namespace koda {

void PhiInstruction::addOption(BasicBlock *incoming_bb, Instruction *value) {
  assert(value->getType() == m_type);
  if (value->getType() != m_type) {
    throw IROperandError("Invalid phi operand type");
  }

  value->addUser(this);

  m_incoming_blocks.push_back(incoming_bb);
  m_values.push_back(value);
}

BasicBlock *BranchInstruction::getTarget() const { return m_bblock->getUncondSuccessor(); }

BasicBlock *ConditionalBranchInstruction::getFalseBLock() const { return m_bblock->getFalseSuccessor(); }

BasicBlock *ConditionalBranchInstruction::getTrueBlock() const { return m_bblock->getTrueSuccessor(); }

} // namespace koda