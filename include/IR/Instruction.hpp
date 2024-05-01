#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>

#include <algorithm>
#include <limits>
#include <ostream>
#include <stddef.h>
#include <vector>

namespace koda {

class BasicBlock;

// Base class for IR instruction
//
class Instruction : public IntrusiveListNode, public IOperand {
protected:
  instid_t m_id;

  InstOpcode m_opcode = INST_INVALID;

  bool m_is_term = false;

  std::vector<Instruction *> m_users{};

  std::vector<Instruction *> m_inputs{};

  BasicBlock *m_bblock = nullptr;

  virtual void dump_(std::ostream &os) const {
    os << operand_type_to_str(get_type());
    for (auto &&input : m_inputs) {
      os << " " << operand_type_to_str(input->get_type()) << " i"
         << input->get_id();
    }
  };

public:
  virtual ~Instruction() = default;

  Instruction(instid_t id, InstOpcode opc)
      : m_id(id), m_opcode(opc), m_is_term(is_terminator_opcode(opc)) {}

  Instruction(const Instruction &) = delete;
  Instruction &operator=(const Instruction &) = delete;

  instid_t get_id() const { return m_id; }

  InstOpcode get_opcode() const { return m_opcode; };

  BasicBlock *get_bb() const { return m_bblock; }

  void set_bb(BasicBlock *bb) { m_bblock = bb; }

  void add_user(Instruction *user) { m_users.push_back(user); }

  size_t get_num_users() const { return m_users.size(); }

  auto users_begin() { return m_users.begin(); }

  auto users_end() { return m_users.end(); }

  auto users_begin() const { return m_users.begin(); }

  auto users_end() const { return m_users.end(); }

  void add_input(Instruction *input) { m_inputs.push_back(input); }

  size_t get_num_inputs() const { return m_inputs.size(); }

  auto inputs_begin() { return m_inputs.begin(); }

  auto inputs_end() { return m_inputs.end(); }

  auto inputs_begin() const { return m_inputs.begin(); }

  auto inputs_end() const { return m_inputs.end(); }

  bool is_terminator() const { return m_is_term; }

  bool is_phi() const { return m_opcode == INST_PHI; }

  bool is_def() { return get_num_users() != 0; }

  void dump(std::ostream &os) const {
    os << "i" << get_id() << ": " << inst_opc_to_str(get_opcode()) << " ";
    dump_(os);
  }
};

struct BranchInstruction : public Instruction {
  BranchInstruction(instid_t id) : Instruction(id, INST_BRANCH) {}

  BasicBlock *get_target() const;

  OperandType get_type() const override { return OperandType::NONE; }

private:
  void dump_(std::ostream &os) const override;
};

class BinaryOpInstructionBase : public Instruction {
  enum SIDE_IDX : unsigned { LHS = 0, RHS = 1 };

public:
  BinaryOpInstructionBase(instid_t id, InstOpcode opc, Instruction *lhs,
                          Instruction *rhs)
      : Instruction(id, opc) {
    m_inputs.resize(2);
    m_inputs[LHS] = lhs;
    m_inputs[RHS] = rhs;
  }

  Instruction *get_lhs() const { return m_inputs[LHS]; }

  Instruction *get_rhs() const { return m_inputs[RHS]; }
};

class ConditionalBranchInstruction : public BinaryOpInstructionBase {
  enum TargetIdx : unsigned { FALSE_IDX = 0, TRUE_IDX = 1 };

  CmpFlag m_flag = CMP_INVALID;

public:
  ConditionalBranchInstruction(instid_t id, CmpFlag flag, Instruction *lhs,
                               Instruction *rhs)
      : BinaryOpInstructionBase(id, INST_COND_BR, lhs, rhs), m_flag(flag) {}

  CmpFlag get_flag() const { return m_flag; }

  BasicBlock *get_false_block() const;

  BasicBlock *get_true_block() const;

  OperandType get_type() const override { return OperandType::NONE; }

private:
  void dump_(std::ostream &os) const override;
};

class ArithmeticInstruction : public BinaryOpInstructionBase {
  OperandType m_type;

public:
  ArithmeticInstruction(instid_t id, InstOpcode opc, OperandType type,
                        Instruction *lhs, Instruction *rhs)
      : BinaryOpInstructionBase(id, opc, lhs, rhs), m_type(type) {}

  OperandType get_type() const override { return m_type; }
};

class PhiInstruction : public Instruction {
  OperandType m_type;
  std::vector<BasicBlock *> m_incoming_blocks;

public:
  PhiInstruction(instid_t id, OperandType type)
      : Instruction(id, INST_PHI), m_type(type) {}

  void add_option(BasicBlock *incoming_bb, Instruction *value);

  OperandType get_type() const override { return m_type; }

  std::pair<BasicBlock *, Instruction *> get_option(size_t idx) {
    return {m_incoming_blocks[idx], m_inputs[idx]};
  }

  Instruction *get_value_for(BasicBlock *bb) {
    assert(m_inputs.size() == m_incoming_blocks.size());
    auto bb_pos =
        std::find(m_incoming_blocks.begin(), m_incoming_blocks.end(), bb);
    if (bb_pos == m_incoming_blocks.end()) {
      return nullptr;
    }
    size_t pos = std::distance(m_incoming_blocks.begin(), bb_pos);
    return m_inputs[pos];
  }

private:
  void dump_(std::ostream &os) const override;
};

class LoadParam : public Instruction {
  OperandType m_type = OperandType::TYPE_INVALID;
  size_t m_index = std::numeric_limits<size_t>::max();

public:
  LoadParam(instid_t id, OperandType type, size_t index)
      : Instruction(id, INST_PARAM), m_type(type), m_index(index) {}

  virtual ~LoadParam() = default;

  OperandType get_type() const override { return m_type; }

  size_t get_index() const { return m_index; }

private:
  void dump_(std::ostream &os) const override {
    os << operand_type_to_str(get_type()) << m_index;
  }
};

template <typename ValueTy> class LoadConstant : public Instruction {
  ValueTy m_value{};
  OperandType m_type = OperandType::TYPE_INVALID;

public:
  LoadConstant(instid_t id, OperandType type, ValueTy value)
      : Instruction(id, INST_CONST), m_value(value), m_type(type) {}

  virtual ~LoadConstant() = default;

  OperandType get_type() const override { return m_type; }

  ValueTy get_value() const { return m_value; }

private:
  void dump_(std::ostream &os) const override {
    os << operand_type_to_str(get_type()) << " " << m_value;
  }
};

class BitOperation : public BinaryOpInstructionBase {
public:
  virtual ~BitOperation() = default;

  BitOperation(instid_t id, InstOpcode opc, Instruction *lhs, Instruction *rhs)
      : BinaryOpInstructionBase(id, opc, lhs, rhs) {
    assert(lhs->get_type() == INTEGER);
    assert(rhs->get_type() == INTEGER);
  }

  OperandType get_type() const override { return INTEGER; }
};

class BitShift : public BitOperation {
public:
  virtual ~BitShift() = default;

  BitShift(instid_t id, InstOpcode opc, Instruction *lhs, Instruction *rhs)
      : BitOperation(id, opc, lhs, rhs) {}

  Instruction *get_value() { return get_lhs(); }

  Instruction *get_shift() { return get_rhs(); }
};

class BitNot : public Instruction {
public:
  virtual ~BitNot() = default;

  BitNot(instid_t id, Instruction *input) : Instruction(id, INST_NOT) {
    assert(input->get_type() == INTEGER);
    m_inputs.resize(1, input);
  }

  OperandType get_type() const override { return INTEGER; }
};

}; // namespace koda