#pragma once

#include <DataStructures/List.hpp>
#include <IR/IROperand.hpp>
#include <IR/IRTypes.hpp>

#include <limits>
#include <stddef.h>
#include <vector>

namespace koda {

class BasicBlock;

// Base class for IR instruction
//
class Instruction : public IntrusiveListNode<Instruction, IntrusiveList<Instruction>>, public IOperand {
  instid_t m_id;

  InstOpcode m_opcode = INST_INVALID;

  BasicBlock *m_bblock = nullptr;

  std::vector<Instruction *> m_users{};

public:
  virtual ~Instruction() = default;

  Instruction(instid_t id, InstOpcode opc) : m_id(id), m_opcode(opc) {}

  instid_t get_id() const { return m_id; }

  InstOpcode getOpcode() const { return m_opcode; };

  BasicBlock *getBB() const { return m_bblock; }

  void setBB(BasicBlock *bb) { m_bblock = bb; }

  void addUser(Instruction *user) { m_users.push_back(user); }

  bool isTerminator() const { return isTerminatorOpcode(m_opcode); }
};

class BranchInstruction : public Instruction {
  BasicBlock *m_target;

public:
  BranchInstruction(instid_t id, BasicBlock *target) : Instruction(id, INST_BRANCH), m_target(target) {}

  BasicBlock *getTarget() const { return m_target; }

  OperandType getType() const override { return OperandType::NONE; }
};

class BinaryOpInstructionBase : public Instruction {
  enum SIDE_IDX : unsigned { LHS = 0, RHS = 1 };

  std::array<IOperand *, 2> m_inputs{};

public:
  BinaryOpInstructionBase(instid_t id, InstOpcode opc, IOperand *lhs, IOperand *rhs)
      : Instruction(id, opc), m_inputs{lhs, rhs} {}

  IOperand *getLhs() const { return m_inputs[LHS]; }

  IOperand *getRhs() const { return m_inputs[RHS]; }
};

class ConditionalBranchInstruction : public BinaryOpInstructionBase {
  enum TargetIdx : unsigned { FALSE_IDX = 0, TRUE_IDX = 1 };

  std::array<BasicBlock *, 2> m_targets = {};

  CmpFlag m_flag = CMP_INVALID;

public:
  ConditionalBranchInstruction(instid_t id, CmpFlag flag, BasicBlock *false_block, BasicBlock *true_block,
                               IOperand *lhs, IOperand *rhs)
      : BinaryOpInstructionBase(id, INST_COND_BR, lhs, rhs), m_targets({false_block, true_block}),
        m_flag(flag) {}

  CmpFlag getFlag() const { return m_flag; }

  BasicBlock *getFalseBLock() const { return m_targets[FALSE_IDX]; }

  BasicBlock *getTrueBlock() const { return m_targets[TRUE_IDX]; }

  OperandType getType() const override { return OperandType::NONE; }
};

class ArithmeticInstruction : public BinaryOpInstructionBase {
  OperandType m_type;

public:
  ArithmeticInstruction(instid_t id, InstOpcode opc, OperandType type, IOperand *lhs, IOperand *rhs)
      : BinaryOpInstructionBase(id, opc, lhs, rhs), m_type(type) {}

  OperandType getType() const override { return m_type; }
};

class PhiNodeInstruction : public Instruction {

  std::vector<IPhiOperand *> m_options;

public:
  PhiNodeInstruction(instid_t id, std::initializer_list<IPhiOperand *> options)
      : Instruction(id, InstOpcode::INST_PHI), m_options{options} {}
};

class LoadParam : public Instruction {
  OperandType m_type = OperandType::TYPE_INVALID;
  size_t m_index = std::numeric_limits<size_t>::max();

public:
  LoadParam(instid_t id, OperandType type, size_t index)
      : Instruction(id, INST_PARAM), m_type(type), m_index(index) {}

  OperandType getType() const override { return m_type; }

  size_t getIndex() const { return m_index; }
};

template <typename ValueTy> class LoadConstant : public Instruction {
  ValueTy m_value{};
  OperandType m_type = OperandType::TYPE_INVALID;

public:
  LoadConstant(instid_t id, OperandType type, ValueTy value)
      : Instruction(id, INST_CONST), m_value(value), m_type(type) {}

  OperandType getType() const override { return m_type; }
  ValueTy getValue() const { return m_value; }
};

}; // namespace koda