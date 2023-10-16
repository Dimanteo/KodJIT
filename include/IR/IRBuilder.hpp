#pragma once

#include <IR/Instruction.hpp>

#include <stdexcept>

namespace koda {

class ProgramGraph;
class BasicBlock;

struct IROperandError : std::runtime_error {
  IROperandError(IOperand *operand, OperandType expected_type);
};

class IRBuilder final {

  ProgramGraph *m_graph;

  // Block where new instructions are inserted.
  BasicBlock *m_insert_bb;

  ArithmeticInstruction *createArithmeticInstruction(InstOpcode opcode, OperandType type, IOperand *lhs,
                                                     IOperand *rhs);

public:
  IRBuilder() = default;

  BasicBlock *createBasicBlock();

  void setInsertPoint(BasicBlock *bb) { m_insert_bb = bb; }

  BasicBlock *getInsertPoint() const { return m_insert_bb; }

  BranchInstruction *createBranch(BasicBlock *target);

  ConditionalBranchInstruction *createConditionalBranch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                        BasicBlock *true_block, IOperand *lhs, IOperand *rhs);

  ArithmeticInstruction *createIAdd(IOperand *lhs, IOperand *rhs) {
    return createArithmeticInstruction(INST_ADD, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createISub(IOperand *lhs, IOperand *rhs) {
    return createArithmeticInstruction(INST_SUB, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createIMul(IOperand *lhs, IOperand *rhs) {
    return createArithmeticInstruction(INST_MUL, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createIDiv(IOperand *lhs, IOperand *rhs) {
    return createArithmeticInstruction(INST_DIV, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createMod(IOperand *lhs, IOperand *rhs) {
    return createArithmeticInstruction(INST_MOD, OperandType::INTEGER, lhs, rhs);
  }
};

} // namespace koda
