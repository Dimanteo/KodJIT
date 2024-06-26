#include "IR/IROperand.hpp"
#include "IR/Instruction.hpp"
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <algorithm>
#include <string>

namespace koda {

std::string
IROperandError::make_error_str(std::initializer_list<IOperand *> received,
                               std::initializer_list<OperandType> expected) {
  std::string msg = "Error - type mismatch. Operand types are :\n";
  for (auto &&op : received) {
    msg.append(operand_type_to_str(op->get_type()));
    msg.append(" ");
  }
  msg.append("\nExpected:\n");
  for (auto &&ty : expected) {
    msg.append(operand_type_to_str(ty));
    msg.append(" ");
  }

  return msg;
}

void IRBuilder::insert_after(Instruction *inst, Instruction *point) {
  auto bb = point->get_bb();
  bb->insert_inst_after(inst, point);
}

void IRBuilder::insert_before(Instruction *inst, Instruction *point) {
  auto bb = point->get_bb();
  bb->insert_inst_before(inst, point);
}

void IRBuilder::move_users(Instruction *from, Instruction *to) {
  std::for_each(from->users_begin(), from->users_end(),
                [from, to](Instruction *use) {
                  to->add_user(use);
                  use->switch_input(from, to);
                });
  from->clear_users();
}

Instruction *IRBuilder::rm_instruction(Instruction *inst) {
  auto &&bb = inst->get_bb();
  std::for_each(inst->inputs_begin(), inst->inputs_end(),
                [inst](Instruction *input) { input->rm_user(inst); });
  std::for_each(
      inst->users_begin(), inst->users_end(),
      [inst](Instruction *user) { user->switch_input(inst, nullptr); });
  return bb->remove_instruction(inst);
}

Instruction *IRBuilder::replace(Instruction *old_inst, Instruction *new_inst) {
  auto &&bb = old_inst->get_bb();
  bb->insert_inst_after(new_inst, old_inst);
  move_users(old_inst, new_inst);
  std::for_each(old_inst->inputs_begin(), old_inst->inputs_end(),
                [old_inst](Instruction *input) { input->rm_user(old_inst); });
  return bb->remove_instruction(old_inst);
}

LoadParam *IRBuilder::create_param_load(size_t param_idx) {
  if (param_idx >= m_graph->get_num_params()) {
    throw IRInvalidArgument("Invalid parameter index");
  }

  auto param = m_graph->get_param(param_idx);
  auto load = m_graph->create_instruction<LoadParam>(param.get_type(),
                                                     param.get_index());
  add_instruction(load);

  return load;
}

LoadConstant<int64_t> *IRBuilder::create_int_constant(int64_t value) {
  auto inst = make_int_constant(value);
  add_instruction(inst);
  return inst;
}

LoadConstant<int64_t> *IRBuilder::make_int_constant(int64_t value) {
  return m_graph->create_instruction<LoadConstant<int64_t>>(
      OperandType::INTEGER, value);
}

BranchInstruction *IRBuilder::create_branch(BasicBlock *target) {
  auto br_inst = m_graph->create_instruction<BranchInstruction>();
  add_instruction(br_inst);

  auto curr_bb = get_insert_point();
  if (!curr_bb->has_successor()) {
    curr_bb->set_uncond_successor(target);
    target->add_predecessor(curr_bb);
  }

  return br_inst;
}

ConditionalBranchInstruction *
IRBuilder::create_conditional_branch(CmpFlag cmp_flag, BasicBlock *false_block,
                                     BasicBlock *true_block, Instruction *lhs,
                                     Instruction *rhs) {
  if (lhs->get_type() != OperandType::INTEGER ||
      rhs->get_type() != OperandType::INTEGER) {
    throw IROperandError(IROperandError::make_error_str(
        {lhs, rhs}, {OperandType::INTEGER, OperandType::INTEGER}));
  }

  auto inst = m_graph->create_instruction<ConditionalBranchInstruction>(
      cmp_flag, lhs, rhs);
  add_instruction(inst);
  add_user_to(inst, {lhs, rhs});

  auto curr_bb = get_insert_point();

  // Update successors only if this is first branch in bb. Otherwise current
  // instruction is unreachable.
  //
  if (!curr_bb->has_successor()) {
    curr_bb->set_cond_successors(false_block, true_block);
    false_block->add_predecessor(curr_bb);
    true_block->add_predecessor(curr_bb);
  }

  return inst;
}

PhiInstruction *IRBuilder::create_phi(OperandType type) {
  auto phi = m_graph->create_instruction<PhiInstruction>(type);
  add_instruction(phi);
  return phi;
}

BitShift *IRBuilder::make_shr(Instruction *lhs, Instruction *rhs) {
  if (lhs->get_type() != INTEGER || rhs->get_type() != INTEGER) {
    auto &&errmsg =
        IROperandError::make_error_str({lhs, rhs}, {INTEGER, INTEGER});
    throw IROperandError(errmsg);
  }
  BitShift *inst = m_graph->create_instruction<BitShift>(INST_SHR, lhs, rhs);
  add_user_to(inst, {lhs, rhs});
  return inst;
}

BitNot *IRBuilder::create_not(Instruction *val) {
  auto bitnot = m_graph->create_instruction<BitNot>(val);
  add_instruction(bitnot);
  add_user_to(bitnot, {val});
  return bitnot;
}

ReturnInstruction *IRBuilder::create_ret(Instruction *val) {
  auto ret = m_graph->create_instruction<ReturnInstruction>(val);
  add_instruction(ret);
  add_user_to(ret, {val});
  return ret;
}

}; // namespace koda