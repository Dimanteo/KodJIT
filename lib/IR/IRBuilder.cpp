#include "IR/IROperand.hpp"
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace koda {

static std::string makeErrorStr(std::initializer_list<IOperand *> received,
                                std::initializer_list<OperandType> expected) {
  std::string msg = "Error - type mismatch. Operand types are :\n";
  for (auto &&op : received) {
    msg.append(OperandTypeToStr[op->getType()]);
    msg.append(" ");
  }
  msg.append("\nExpected:\n");
  for (auto &&ty : expected) {
    msg.append(OperandTypeToStr[ty]);
    msg.append(" ");
  }

  return msg;
}

BasicBlock *IRBuilder::createBasicBlock() { return m_graph->createBasicBlock(); }

ProgParam *IRBuilder::appendProgParam(OperandType type) {
  m_graph->appendParam(type);
  return m_graph->paramBack();
}

void IRBuilder::popProgParam() { m_graph->popParam(); }

ProgParam *IRBuilder::getProgParam(size_t idx) const { return m_graph->getParam(idx); }

IntConstOperand *IRBuilder::createIntConstant(uint64_t value) { return m_graph->createIntConstant(value); }

static void connectUsers(Instruction *user, std::vector<IOperand *> sources) {
  std::for_each(sources.begin(), sources.end(), [user](IOperand *src) {
    if (src->isTrackingUsers()) {
      src->cast_to<Instruction>()->addUser(user);
    }
  });
}

ArithmeticInstruction *IRBuilder::createArithmeticInstruction(InstOpcode opcode, OperandType type,
                                                              IOperand *lhs, IOperand *rhs) {
  if (lhs->getType() != type || rhs->getType() != type) {
    throw IROperandError(makeErrorStr({lhs, rhs}, {type, type}));
  }

  auto inst = m_graph->createInstruction<ArithmeticInstruction>(opcode, type, lhs, rhs);
  connectUsers(inst, {lhs, rhs});
  return inst;
}

BranchInstruction *IRBuilder::createBranch(BasicBlock *target) {
  auto br_inst = m_graph->createInstruction<BranchInstruction>(target);
  auto curr_bb = getInsertPoint();
  if (!curr_bb->hasTerminator()) {
    curr_bb->setUncondSuccessor(target);
  }
  return br_inst;
}

ConditionalBranchInstruction *IRBuilder::createConditionalBranch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                                 BasicBlock *true_block, IOperand *lhs,
                                                                 IOperand *rhs) {
  if (lhs->getType() != OperandType::INTEGER || rhs->getType() != OperandType::INTEGER) {
    throw IROperandError(makeErrorStr({lhs, rhs}, {OperandType::INTEGER, OperandType::INTEGER}));
  }

  auto inst =
      m_graph->createInstruction<ConditionalBranchInstruction>(cmp_flag, false_block, true_block, lhs, rhs);

  connectUsers(inst, {lhs, rhs});

  auto curr_bb =  getInsertPoint();

  // Update successors only if this is first branch in bb. Otherwise current instruction is unreachable.
  //
  if (!curr_bb->hasTerminator()) {
    curr_bb->setFalseSuccessor(false_block);
    curr_bb->setTrueSuccessor(true_block);
  }

  return inst;
}

}; // namespace koda