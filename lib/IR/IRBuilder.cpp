#include "IR/IROperand.hpp"
#include "IR/Instruction.hpp"
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace koda {

static std::string makeErrorStr(std::initializer_list<IOperand *> received,
                                std::initializer_list<OperandType> expected) {
  std::string msg = "Error - type mismatch. Operand types are :\n";
  for (auto &&op : received) {
    msg.append(operandTypeToStr(op->getType()));
    msg.append(" ");
  }
  msg.append("\nExpected:\n");
  for (auto &&ty : expected) {
    msg.append(operandTypeToStr(ty));
    msg.append(" ");
  }

  return msg;
}

static void addUserTo(Instruction *user, std::vector<Instruction *> sources) {
  std::for_each(sources.begin(), sources.end(), [user](Instruction *src) { src->addUser(user); });
}

LoadParam *IRBuilder::createParamLoad(size_t param_idx) {
  if (param_idx >= m_graph->getNumParams()) {
    throw IRInvalidArgument("Invalid parameter index");
  }

  auto param = m_graph->getParam(param_idx);
  auto load = m_graph->createInstruction<LoadParam>(param.getType(), param.getIndex());
  addInstruction(load);

  return load;
}

LoadConstant<int64_t> *IRBuilder::createIntConstant(int64_t value) {
  auto inst = m_graph->createInstruction<LoadConstant<int64_t>>(OperandType::INTEGER, value);
  addInstruction(inst);
  return inst;
}

ArithmeticInstruction *IRBuilder::createArithmeticInstruction(InstOpcode opcode, OperandType type,
                                                              Instruction *lhs, Instruction *rhs) {
  if (lhs->getType() != type || rhs->getType() != type) {
    throw IROperandError(makeErrorStr({lhs, rhs}, {type, type}));
  }

  auto inst = m_graph->createInstruction<ArithmeticInstruction>(opcode, type, lhs, rhs);
  addInstruction(inst);
  addUserTo(inst, {lhs, rhs});
  return inst;
}

BranchInstruction *IRBuilder::createBranch(BasicBlock *target) {
  auto br_inst = m_graph->createInstruction<BranchInstruction>();
  addInstruction(br_inst);

  auto curr_bb = getInsertPoint();
  if (!curr_bb->hasSuccessor()) {
    curr_bb->setUncondSuccessor(target);
    target->addPredecessor(curr_bb);
  }

  return br_inst;
}

ConditionalBranchInstruction *IRBuilder::createConditionalBranch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                                 BasicBlock *true_block, Instruction *lhs,
                                                                 Instruction *rhs) {
  if (lhs->getType() != OperandType::INTEGER || rhs->getType() != OperandType::INTEGER) {
    throw IROperandError(makeErrorStr({lhs, rhs}, {OperandType::INTEGER, OperandType::INTEGER}));
  }

  auto inst = m_graph->createInstruction<ConditionalBranchInstruction>(cmp_flag, lhs, rhs);
  addInstruction(inst);
  addUserTo(inst, {lhs, rhs});

  auto curr_bb = getInsertPoint();

  // Update successors only if this is first branch in bb. Otherwise current instruction is unreachable.
  //
  if (!curr_bb->hasSuccessor()) {
    curr_bb->setCondSuccessors(false_block, true_block);
    false_block->addPredecessor(curr_bb);
    true_block->addPredecessor(curr_bb);
  }

  return inst;
}

PhiInstruction *IRBuilder::createPHI(OperandType type) {
  auto phi = m_graph->createInstruction<PhiInstruction>(type);
  addInstruction(phi);
  return phi;
}

}; // namespace koda