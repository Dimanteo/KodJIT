#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>
#include <IR/Instruction.hpp>

namespace koda {

void Instruction::rm_user(Instruction *user) {
  auto pos = std::find(m_users.begin(), m_users.end(), user);
  m_users.erase(pos);
}

void Instruction::rm_user(size_t idx) {
  auto pos = m_users.begin();
  std::advance(pos, idx);
  m_users.erase(pos);
}

void Instruction::switch_input(Instruction *oldin, Instruction *newin) {
  std::replace(m_inputs.begin(), m_inputs.end(), oldin, newin);
}

void PhiInstruction::add_option(BasicBlock *incoming_bb, Instruction *value) {
  assert(value->get_type() == m_type);
  if (value->get_type() != m_type) {
    throw IROperandError("Invalid phi operand type");
  }

  value->add_user(this);

  m_incoming_blocks.push_back(incoming_bb);
  m_inputs.push_back(value);
}

BasicBlock *BranchInstruction::get_target() const {
  return m_bblock->get_uncond_successor();
}

BasicBlock *ConditionalBranchInstruction::get_false_block() const {
  return m_bblock->get_false_successor();
}

BasicBlock *ConditionalBranchInstruction::get_true_block() const {
  return m_bblock->get_true_successor();
}

void BranchInstruction::dump_(std::ostream &os) const {
  os << "bb" << get_target()->get_id();
}

void ConditionalBranchInstruction::dump_(std::ostream &os) const {
  os << flag_to_str(m_flag) << " ";
  os << "i" << get_lhs()->get_id() << ", i" << get_rhs()->get_id() << " ";
  os << "F: bb" << get_false_block()->get_id() << " T: bb"
     << get_true_block()->get_id();
}

void PhiInstruction::dump_(std::ostream &os) const {
  os << operand_type_to_str(get_type());
  for (size_t i = 0; i < m_incoming_blocks.size(); ++i) {
    auto bb = m_incoming_blocks[i];
    auto inst = m_inputs[i];
    os << " [" << i << ": bb" << bb->get_id() << " i" << inst->get_id()
       << "]; ";
  }
}

} // namespace koda