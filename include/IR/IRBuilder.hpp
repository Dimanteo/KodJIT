#pragma once

#include "IR/IROperand.hpp"
#include <IR/Instruction.hpp>
#include <IR/ProgramGraph.hpp>

#include <initializer_list>
#include <stdexcept>
#include <vector>

namespace koda {

class ProgramGraph;
class BasicBlock;

struct IROperandError : std::runtime_error {
  IROperandError(const char *msg) : std::runtime_error(msg) {}
  IROperandError(const std::string msg) : std::runtime_error(msg) {}
  static const char *makeErrorStr(std::initializer_list<IOperand> ops, OperandType expected);
};

struct IRInvalidArgument : std::runtime_error {
  IRInvalidArgument(const char *msg) : std::runtime_error(msg) {}
};

class IRBuilder final {

  ProgramGraph *m_graph;

  // Block where new instructions are inserted.
  BasicBlock *m_insert_bb;

  ArithmeticInstruction *createArithmeticInstruction(InstOpcode opcode, OperandType type, Instruction *lhs,
                                                     Instruction *rhs);

  void addInstruction(Instruction *inst) { m_insert_bb->addInstruction(inst); }

public:
  IRBuilder(ProgramGraph &graph) : m_graph(&graph) {}

  void setEntryPoint(BasicBlock *bb) { m_graph->setEntry(bb); }

  void setInsertPoint(BasicBlock *bb) { m_insert_bb = bb; }

  BasicBlock *getInsertPoint() const { return m_insert_bb; }

  LoadParam *createParamLoad(size_t param_idx);

  LoadConstant<int64_t> *createIntConstant(int64_t value);

  BranchInstruction *createBranch(BasicBlock *target);

  ConditionalBranchInstruction *createConditionalBranch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                        BasicBlock *true_block, Instruction *lhs,
                                                        Instruction *rhs);

  ArithmeticInstruction *createIAdd(Instruction *lhs, Instruction *rhs) {
    return createArithmeticInstruction(INST_ADD, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createISub(Instruction *lhs, Instruction *rhs) {
    return createArithmeticInstruction(INST_SUB, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createIMul(Instruction *lhs, Instruction *rhs) {
    return createArithmeticInstruction(INST_MUL, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createIDiv(Instruction *lhs, Instruction *rhs) {
    return createArithmeticInstruction(INST_DIV, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *createMod(Instruction *lhs, Instruction *rhs) {
    return createArithmeticInstruction(INST_MOD, OperandType::INTEGER, lhs, rhs);
  }

  PhiInstruction *createPHI(OperandType type);
};

} // namespace koda
