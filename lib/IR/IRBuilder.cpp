#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <algorithm>

namespace koda {

BasicBlock *IRBuilder::createBasicBlock() { return m_graph->createBasicBlock(); }

static void connectUsers(Instruction *user, std::vector<IOperand *> sources) {
  std::for_each(sources.begin(), sources.end(), [user](IOperand *src) {
    if (src->isTrackingUsers()) {
      src->cast_to<IInstructionOperand>()->addUser(user);
    }
  });
}

ArithmeticInstruction *IRBuilder::createArithmeticInstruction(InstOpcode opcode, OperandType type,
                                                              IOperand *lhs, IOperand *rhs) {
  if (lhs->getType() != type || rhs->getType() != type) {
    throw IROperandError(lhs->getType() != type ? lhs : rhs, type);
  }
  auto inst = m_graph->createInstruction<ArithmeticInstruction>(opcode, type, lhs, rhs);
  connectUsers(inst, {lhs, rhs});
  return inst;
}

BranchInstruction *IRBuilder::createBranch(BasicBlock *target) {
  return m_graph->createInstruction<BranchInstruction>(target);
}

ConditionalBranchInstruction *IRBuilder::createConditionalBranch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                                 BasicBlock *true_block, IOperand *lhs,
                                                                 IOperand *rhs) {
  if (lhs->getType() != OperandType::INTEGER || rhs->getType() != OperandType::INTEGER) {
    throw IROperandError(lhs->getType() != OperandType::INTEGER ? lhs : rhs, OperandType::INTEGER);
  }
  auto inst =
      m_graph->createInstruction<ConditionalBranchInstruction>(cmp_flag, false_block, true_block, lhs, rhs);
  connectUsers(inst, {lhs, rhs});
  return inst;
}

}; // namespace koda