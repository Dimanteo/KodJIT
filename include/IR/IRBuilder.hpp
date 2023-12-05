#pragma once

#include "IR/IROperand.hpp"
#include <IR/Instruction.hpp>
#include <IR/ProgramGraph.hpp>

#include <initializer_list>
#include <stdexcept>

namespace koda {

class ProgramGraph;
class BasicBlock;

struct IROperandError : std::runtime_error {
  IROperandError(const char *msg) : std::runtime_error(msg) {}
  IROperandError(const std::string msg) : std::runtime_error(msg) {}
  static const char *make_error_str(std::initializer_list<IOperand> ops, OperandType expected);
};

struct IRInvalidArgument : std::runtime_error {
  IRInvalidArgument(const char *msg) : std::runtime_error(msg) {}
};

class IRBuilder final {

  ProgramGraph *m_graph;

  // Block where new instructions are inserted.
  BasicBlock *m_insert_bb;

  ArithmeticInstruction *create_arithmetic_instruction(InstOpcode opcode, OperandType type, Instruction *lhs,
                                                       Instruction *rhs);

  void add_instruction(Instruction *inst) { m_insert_bb->add_instruction(inst); }

public:
  IRBuilder(ProgramGraph &graph) : m_graph(&graph) {}

  void set_entry_point(BasicBlock *bb) { m_graph->set_entry(bb); }

  void set_insert_point(BasicBlock *bb) { m_insert_bb = bb; }

  BasicBlock *get_insert_point() const { return m_insert_bb; }

  LoadParam *create_param_load(size_t param_idx);

  LoadConstant<int64_t> *create_int_constant(int64_t value);

  BranchInstruction *create_branch(BasicBlock *target);

  ConditionalBranchInstruction *create_conditional_branch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                          BasicBlock *true_block, Instruction *lhs,
                                                          Instruction *rhs);

  ArithmeticInstruction *create_iadd(Instruction *lhs, Instruction *rhs) {
    return create_arithmetic_instruction(INST_ADD, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_isub(Instruction *lhs, Instruction *rhs) {
    return create_arithmetic_instruction(INST_SUB, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_imul(Instruction *lhs, Instruction *rhs) {
    return create_arithmetic_instruction(INST_MUL, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_idiv(Instruction *lhs, Instruction *rhs) {
    return create_arithmetic_instruction(INST_DIV, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_mod(Instruction *lhs, Instruction *rhs) {
    return create_arithmetic_instruction(INST_MOD, OperandType::INTEGER, lhs, rhs);
  }

  PhiInstruction *create_phi(OperandType type);
};

} // namespace koda
