#include <IR/BasicBlock.hpp>
#include <IR/Instruction.hpp>
#include <IR/IRBuilder.hpp>

namespace koda {

void PhiInstruction::addOption(BasicBlock *incoming_bb, Instruction *value) {
  assert(value->getType() == m_type);
  if (value->getType() != m_type) {
    throw IROperandError("Invalid phi operand type");
  }

  value->addUser(this);

  m_incoming_blocks.push_back(incoming_bb);
  m_values.push_back(value);
}

BasicBlock *BranchInstruction::getTarget() const { return m_bblock->getUncondSuccessor(); }

BasicBlock *ConditionalBranchInstruction::getFalseBLock() const { return m_bblock->getFalseSuccessor(); }

BasicBlock *ConditionalBranchInstruction::getTrueBlock() const { return m_bblock->getTrueSuccessor(); }

void BranchInstruction::dump_(std::ostream &os) const {
  os << "b " << "bb" << getTarget()->getID();
}

void ConditionalBranchInstruction::dump_(std::ostream &os) const {
  os << "b." << FlagToStr[m_flag] << " ";
  os << "i" << getLhs()->getID() << ", i" << getRhs()->getID() << " ";
  os << "F: bb" << getFalseBLock()->getID() << " T: bb" << getTrueBlock()->getID();
}

void ArithmeticInstruction::dump_(std::ostream &os) const {
  os << OperandTypeToStr[getType()] << " ";
  switch (getOpcode()) {
    case INST_ADD:
      os << "add";
      break;
    case INST_MUL:
      os << "mul";
      break;
    case INST_SUB:
      os << "sub";
      break;
    case INST_DIV:
      os << "div";
      break;
    case INST_MOD:
      os << "mod";
      break;
    default:
      os << "UNDEF";
      break;
  }
  auto dump_arg = [](std::ostream &outs, Instruction *arg) {
    outs << " " << OperandTypeToStr[arg->getType()] << " i" << arg->getID();
  };
  dump_arg(os, getLhs());
  dump_arg(os, getRhs());
}

void PhiInstruction::dump_(std::ostream &os) const {
  os << "phi " << OperandTypeToStr[m_type];
  for (size_t i = 0; i < m_incoming_blocks.size(); ++i) {
    auto bb = m_incoming_blocks[i];
    auto inst = m_values[i];
    os << " [" << i <<  ": bb" << bb->getID() << " i" << inst->getID() << "]; ";
  }
}

} // namespace koda