#include "IR/IROperand.hpp"
#include "IR/Instruction.hpp"
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <algorithm>
#include <string>

namespace koda {

static std::string makeErrorStr(std::initializer_list<IOperand *> received,
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

static void add_user_to(Instruction *user, std::vector<Instruction *> sources) {
  std::for_each(sources.begin(), sources.end(), [user](Instruction *src) { src->add_user(user); });
}

LoadParam *IRBuilder::create_param_load(size_t param_idx) {
  if (param_idx >= m_graph->get_num_params()) {
    throw IRInvalidArgument("Invalid parameter index");
  }

  auto param = m_graph->get_param(param_idx);
  auto load = m_graph->create_instruction<LoadParam>(param.get_type(), param.get_index());
  add_instruction(load);

  return load;
}

LoadConstant<int64_t> *IRBuilder::create_int_constant(int64_t value) {
  auto inst = m_graph->create_instruction<LoadConstant<int64_t>>(OperandType::INTEGER, value);
  add_instruction(inst);
  return inst;
}

ArithmeticInstruction *IRBuilder::create_arithmetic_instruction(InstOpcode opcode, OperandType type,
                                                                Instruction *lhs, Instruction *rhs) {
  if (lhs->get_type() != type || rhs->get_type() != type) {
    throw IROperandError(makeErrorStr({lhs, rhs}, {type, type}));
  }

  auto inst = m_graph->create_instruction<ArithmeticInstruction>(opcode, type, lhs, rhs);
  add_instruction(inst);
  add_user_to(inst, {lhs, rhs});
  return inst;
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

ConditionalBranchInstruction *IRBuilder::create_conditional_branch(CmpFlag cmp_flag, BasicBlock *false_block,
                                                                   BasicBlock *true_block, Instruction *lhs,
                                                                   Instruction *rhs) {
  if (lhs->get_type() != OperandType::INTEGER || rhs->get_type() != OperandType::INTEGER) {
    throw IROperandError(makeErrorStr({lhs, rhs}, {OperandType::INTEGER, OperandType::INTEGER}));
  }

  auto inst = m_graph->create_instruction<ConditionalBranchInstruction>(cmp_flag, lhs, rhs);
  add_instruction(inst);
  add_user_to(inst, {lhs, rhs});

  auto curr_bb = get_insert_point();

  // Update successors only if this is first branch in bb. Otherwise current instruction is unreachable.
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

}; // namespace koda