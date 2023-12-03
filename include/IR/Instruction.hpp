#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>

#include <limits>
#include <ostream>
#include <stddef.h>
#include <vector>

namespace koda {

class BasicBlock;

// Base class for IR instruction
//
class Instruction : public IntrusiveListNode, public IOperand {
  instid_t m_id;

  InstOpcode m_opcode = INST_INVALID;

  bool m_is_term = false;

  std::vector<Instruction *> m_users{};

protected:
  BasicBlock *m_bblock = nullptr;

  virtual void dump_(std::ostream &os) const { (void)os; };

public:
  virtual ~Instruction() = default;

  Instruction(instid_t id, InstOpcode opc) : m_id(id), m_opcode(opc), m_is_term(isTerminatorOpcode(opc)) {}

  Instruction(const Instruction &) = delete;
  Instruction &operator=(const Instruction &) = delete;

  instid_t getID() const { return m_id; }

  InstOpcode getOpcode() const { return m_opcode; };

  BasicBlock *getBB() const { return m_bblock; }

  void setBB(BasicBlock *bb) { m_bblock = bb; }

  void addUser(Instruction *user) { m_users.push_back(user); }

  bool isTerminator() const { return m_is_term; }

  void dump(std::ostream &os) const {
    os << "i" << getID() << ": " << instOpcToStr(getOpcode()) << " ";
    dump_(os);
  }
};

struct BranchInstruction : public Instruction {
  BranchInstruction(instid_t id) : Instruction(id, INST_BRANCH) {}

  BasicBlock *getTarget() const;

  OperandType getType() const override { return OperandType::NONE; }

private:
  void dump_(std::ostream &os) const override;
};

class BinaryOpInstructionBase : public Instruction {
  enum SIDE_IDX : unsigned { LHS = 0, RHS = 1 };

  std::array<Instruction *, 2> m_inputs{};

public:
  BinaryOpInstructionBase(instid_t id, InstOpcode opc, Instruction *lhs, Instruction *rhs)
      : Instruction(id, opc), m_inputs{lhs, rhs} {}

  Instruction *getLhs() const { return m_inputs[LHS]; }

  Instruction *getRhs() const { return m_inputs[RHS]; }
};

class ConditionalBranchInstruction : public BinaryOpInstructionBase {
  enum TargetIdx : unsigned { FALSE_IDX = 0, TRUE_IDX = 1 };

  CmpFlag m_flag = CMP_INVALID;

public:
  ConditionalBranchInstruction(instid_t id, CmpFlag flag, Instruction *lhs, Instruction *rhs)
      : BinaryOpInstructionBase(id, INST_COND_BR, lhs, rhs), m_flag(flag) {}

  CmpFlag getFlag() const { return m_flag; }

  BasicBlock *getFalseBLock() const;

  BasicBlock *getTrueBlock() const;

  OperandType getType() const override { return OperandType::NONE; }

private:
  void dump_(std::ostream &os) const override;
};

class ArithmeticInstruction : public BinaryOpInstructionBase {
  OperandType m_type;

public:
  ArithmeticInstruction(instid_t id, InstOpcode opc, OperandType type, Instruction *lhs, Instruction *rhs)
      : BinaryOpInstructionBase(id, opc, lhs, rhs), m_type(type) {}

  OperandType getType() const override { return m_type; }

private:
  void dump_(std::ostream &os) const override;
};

class PhiInstruction : public Instruction {
  OperandType m_type;
  std::vector<BasicBlock *> m_incoming_blocks;
  std::vector<Instruction *> m_values;

public:
  PhiInstruction(instid_t id, OperandType type) : Instruction(id, INST_PHI), m_type(type) {}

  void addOption(BasicBlock *incoming_bb, Instruction *value);

  OperandType getType() const override { return m_type; }

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

  OperandType getType() const override { return m_type; }

  size_t getIndex() const { return m_index; }

private:
  void dump_(std::ostream &os) const override { os << operandTypeToStr(getType()) << m_index; }
};

template <typename ValueTy> class LoadConstant : public Instruction {
  ValueTy m_value{};
  OperandType m_type = OperandType::TYPE_INVALID;

public:
  LoadConstant(instid_t id, OperandType type, ValueTy value)
      : Instruction(id, INST_CONST), m_value(value), m_type(type) {}

  virtual ~LoadConstant() = default;

  OperandType getType() const override { return m_type; }

  ValueTy getValue() const { return m_value; }

private:
  void dump_(std::ostream &os) const override { os << operandTypeToStr(getType()) << " " << m_value; }
};

}; // namespace koda