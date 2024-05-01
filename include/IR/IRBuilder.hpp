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
  static std::string
  make_error_str(std::initializer_list<IOperand *> ops,
                 std::initializer_list<OperandType> expected);
};

struct IRInvalidArgument : std::runtime_error {
  IRInvalidArgument(const char *msg) : std::runtime_error(msg) {}
};

class IRBuilder final {

  ProgramGraph *m_graph;

  // Block where new instructions are inserted.
  BasicBlock *m_insert_bb;

  template <typename InstT, OperandType OutType = OperandType::TYPE_INVALID>
  InstT *create_binary_op(InstOpcode opcode, OperandType type, Instruction *lhs,
                          Instruction *rhs) {
    if (lhs->get_type() != type || rhs->get_type() != type) {
      auto &&errmsg = IROperandError::make_error_str({lhs, rhs}, {type, type});
      throw IROperandError(errmsg);
    }
    InstT *inst = nullptr;
    if constexpr (OutType == OperandType::TYPE_INVALID) {
      inst = m_graph->create_instruction<InstT>(opcode, lhs, rhs);
    } else {
      inst = m_graph->create_instruction<InstT>(opcode, OutType, lhs, rhs);
    }
    add_instruction(inst);
    add_user_to(inst, {lhs, rhs});
    return inst;
  }

  void add_instruction(Instruction *inst) {
    m_insert_bb->add_instruction(inst);
  }

  static void add_user_to(Instruction *user,
                          std::vector<Instruction *> sources) {
    std::for_each(sources.begin(), sources.end(),
                  [user](Instruction *src) { src->add_user(user); });
  }

public:
  IRBuilder(ProgramGraph &graph) : m_graph(&graph) {}

  void set_entry_point(BasicBlock *bb) { m_graph->set_entry(bb); }

  void set_insert_point(BasicBlock *bb) { m_insert_bb = bb; }

  BasicBlock *get_insert_point() const { return m_insert_bb; }

  static void move_users(Instruction *from, Instruction *to);

  static Instruction *rm_instruction(Instruction *inst);

  static Instruction *replace(Instruction *old_inst, Instruction *new_inst);

  LoadParam *create_param_load(size_t param_idx);

  LoadConstant<int64_t> *create_int_constant(int64_t value);

  // Same as create_ but doesn't add instruction to the basic block
  LoadConstant<int64_t> *make_int_constant(int64_t value);

  BranchInstruction *create_branch(BasicBlock *target);

  ConditionalBranchInstruction *
  create_conditional_branch(CmpFlag cmp_flag, BasicBlock *false_block,
                            BasicBlock *true_block, Instruction *lhs,
                            Instruction *rhs);

  ArithmeticInstruction *create_iadd(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<ArithmeticInstruction, OperandType::INTEGER>(
        INST_ADD, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_isub(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<ArithmeticInstruction, OperandType::INTEGER>(
        INST_SUB, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_imul(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<ArithmeticInstruction, OperandType::INTEGER>(
        INST_MUL, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_idiv(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<ArithmeticInstruction, OperandType::INTEGER>(
        INST_DIV, OperandType::INTEGER, lhs, rhs);
  }

  ArithmeticInstruction *create_mod(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<ArithmeticInstruction, OperandType::INTEGER>(
        INST_MOD, OperandType::INTEGER, lhs, rhs);
  }

  PhiInstruction *create_phi(OperandType type);

  BitShift *create_shl(Instruction *val, Instruction *shift) {
    return create_binary_op<BitShift>(INST_SHL, OperandType::INTEGER, val,
                                      shift);
  }

  BitShift *create_shr(Instruction *val, Instruction *shift) {
    return create_binary_op<BitShift>(INST_SHR, OperandType::INTEGER, val,
                                      shift);
  }

  BitOperation *create_and(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<BitOperation>(INST_AND, OperandType::INTEGER, lhs,
                                          rhs);
  }

  BitOperation *create_or(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<BitOperation>(INST_OR, OperandType::INTEGER, lhs,
                                          rhs);
  }

  BitOperation *create_xor(Instruction *lhs, Instruction *rhs) {
    return create_binary_op<BitOperation>(INST_XOR, OperandType::INTEGER, lhs,
                                          rhs);
  }

  BitNot *create_not(Instruction *val);

  ReturnInstruction *create_ret(Instruction *val);
};

} // namespace koda
