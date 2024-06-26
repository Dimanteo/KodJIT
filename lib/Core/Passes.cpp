#include <Core/Analysis.hpp>
#include <Core/Compiler.h>
#include <Core/Passes.hpp>
#include <DataStructures/Graph.hpp>
#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>

namespace koda {

void RmUnused::run(Compiler &compiler) {
  IRBuilder builder(compiler.graph());
  for (auto &&bb : compiler.graph()) {
    if (bb.empty()) {
      continue;
    }
    Instruction *inst = &*bb.begin();
    while (inst->has_next()) {
      if (inst->get_num_users() == 0 && !inst->has_side_effects()) {
        inst = builder.rm_instruction(inst);
      } else {
        inst = inst->get_next();
      }
    }
  }
}

ConstantFolding::ConstantFolding() {
  auto get_inputs = [](const Instruction &inst) {
    assert(inst.get_num_inputs() == 2 && "Binary operation expected");
    const auto &binop = reinterpret_cast<const BinaryOpInstructionBase &>(inst);
    ConstInst *lhs = reinterpret_cast<ConstInst *>(binop.get_lhs());
    assert(lhs->get_opcode() == INST_CONST && "Can't fold non const value");
    ConstInst *rhs = reinterpret_cast<ConstInst *>(binop.get_rhs());
    assert(rhs->get_opcode() == INST_CONST && "Can't fold non const value");
    return std::make_pair(lhs, rhs);
  };

  m_evaluators[INST_ADD] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_ADD);
    auto &&[lhs, rhs] = get_inputs(inst);
    int64_t result = lhs->get_value() + rhs->get_value();
    return result;
  };
  m_evaluators[INST_SUB] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_SUB);
    auto &&[lhs, rhs] = get_inputs(inst);
    int64_t result = lhs->get_value() - rhs->get_value();
    return result;
  };
  m_evaluators[INST_MUL] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_MUL);
    auto &&[lhs, rhs] = get_inputs(inst);
    int64_t result = lhs->get_value() * rhs->get_value();
    return result;
  };
  m_evaluators[INST_DIV] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_DIV);
    auto &&[lhs, rhs] = get_inputs(inst);
    int64_t result = lhs->get_value() / rhs->get_value();
    return result;
  };
  m_evaluators[INST_SHL] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_SHL);
    auto &&[lhs, rhs] = get_inputs(inst);
    uint64_t result = static_cast<uint64_t>(lhs->get_value())
                      << static_cast<uint64_t>(rhs->get_value());
    return result;
  };
  m_evaluators[INST_SHR] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_SHR);
    auto &&[lhs, rhs] = get_inputs(inst);
    uint64_t result = static_cast<uint64_t>(lhs->get_value()) >>
                      static_cast<uint64_t>(rhs->get_value());
    return result;
  };
  m_evaluators[INST_AND] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_AND);
    auto &&[lhs, rhs] = get_inputs(inst);
    uint64_t result = static_cast<uint64_t>(lhs->get_value()) &
                      static_cast<uint64_t>(rhs->get_value());
    return result;
  };
  m_evaluators[INST_OR] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_OR);
    auto &&[lhs, rhs] = get_inputs(inst);
    uint64_t result = static_cast<uint64_t>(lhs->get_value()) |
                      static_cast<uint64_t>(rhs->get_value());
    return result;
  };
  m_evaluators[INST_XOR] = [&get_inputs](const Instruction &inst) {
    assert(inst.get_opcode() == INST_XOR);
    auto &&[lhs, rhs] = get_inputs(inst);
    uint64_t result = static_cast<uint64_t>(lhs->get_value()) ^
                      static_cast<uint64_t>(rhs->get_value());
    return result;
  };
  m_evaluators[INST_NOT] = [](const Instruction &inst) {
    assert(inst.get_opcode() == INST_NOT);
    ConstInst *val = reinterpret_cast<ConstInst *>(inst.get_input(0));
    assert(val->get_opcode() == INST_CONST && "Can't fold non const value");
    return ~static_cast<uint64_t>(val->get_value());
  };
}

void ConstantFolding::run(Compiler &compiler) {
  auto &&rpo = compiler.get_or_create<RPOAnalysis>(compiler);
  IRBuilder builder(compiler.graph());
  for (auto &&bbid : rpo) {
    auto &&bb = *compiler.graph().get_bb(bbid);
    if (bb.empty()) {
      continue;
    }
    Instruction *inst = &*bb.begin();
    while (inst->has_next()) {
      if (!is_computable(*inst) || !has_const_input(*inst)) {
        inst = inst->get_next();
        continue;
      }
      auto result = fold(*inst);
      auto folded_inst = builder.make_int_constant(result);
      inst = builder.replace(inst, folded_inst);
    }
  }
}

bool ConstantFolding::is_computable(const Instruction &inst) {
  if (inst.get_type() != INTEGER) {
    return false;
  }
  bool is_computable = false;
  switch (inst.get_opcode()) {
  case INST_ADD:
  case INST_SUB:
  case INST_MUL:
  case INST_DIV:
  case INST_SHL:
  case INST_SHR:
  case INST_AND:
  case INST_OR:
  case INST_XOR:
  case INST_NOT:
    is_computable = true;
    break;
  default:
    is_computable = false;
  }
  return is_computable;
}

bool ConstantFolding::has_const_input(const Instruction &inst) {
  return std::all_of(inst.inputs_begin(), inst.inputs_end(),
                     [](const Instruction *input) {
                       return input->get_opcode() == INST_CONST;
                     });
}

int64_t ConstantFolding::fold(const Instruction &act) {
  auto &&eval = m_evaluators[act.get_opcode()];
  auto folded = eval(act);
  return folded;
}

void Peephole::run(Compiler &compiler) {
  auto &&rpo = compiler.get_or_create<RPOAnalysis>(compiler);
  IRBuilder builder(compiler.graph());
  for (auto &&bbid : rpo) {
    auto &&bb = *compiler.graph().get_bb(bbid);
    if (bb.empty()) {
      continue;
    }
    for (auto inst = bb.begin(); inst != bb.end();) {
      auto maybe_next = peephole_and(builder, &*inst);
      if (maybe_next) {
        // After peephole iterator on removed instruction will be invalidated.
        inst = BasicBlock::iterator(maybe_next.value());
        // Restart all checks for next instruction
        continue;
      }
      maybe_next = peephole_sub(builder, &*inst);
      if (maybe_next) {
        inst = BasicBlock::iterator(maybe_next.value());
        continue;
      }
      maybe_next = peephole_shr(builder, &*inst);
      if (maybe_next) {
        inst = BasicBlock::iterator(maybe_next.value());
        continue;
      }
      maybe_next = peephole_div(builder, &*inst);
      if (maybe_next) {
        inst = BasicBlock::iterator(maybe_next.value());
        continue;
      }
      // If no peephole applied go to next instruction
      ++inst;
    }
  }
}

std::optional<Instruction *> Peephole::peephole_and(IRBuilder &builder,
                                                    Instruction *inst) {
  auto apply_peephole = [&builder](BitOperation *and_inst,
                                   Instruction *replacement) {
    builder.move_users(and_inst, replacement);
    auto next = builder.rm_instruction(and_inst);
    return next;
  };
  if (inst->get_opcode() != INST_AND) {
    return std::nullopt;
  }
  auto lhs = inst->get_input(BinaryOpInstructionBase::LHS);
  auto rhs = inst->get_input(BinaryOpInstructionBase::RHS);
  if (lhs->get_id() == rhs->get_id()) {
    // x & x -> x
    return apply_peephole(reinterpret_cast<BitOperation *>(inst), lhs);
  }
  if (lhs->get_opcode() != INST_CONST && rhs->get_opcode() != INST_CONST) {
    return std::nullopt;
  }
  auto &&[var_input, const_input] = std::make_pair(lhs, rhs);
  if (const_input->get_opcode() != INST_CONST) {
    std::swap(const_input, var_input);
  }
  if (is_const_eq(const_input, 0)) {
    // x & 0 -> 0
    return apply_peephole(reinterpret_cast<BitOperation *>(inst), const_input);
  } else if (is_const_eq(const_input, static_cast<int64_t>(~0ul))) {
    // x & 0xFFFF... -> x
    return apply_peephole(reinterpret_cast<BitOperation *>(inst), var_input);
  }
  return inst;
}

std::optional<Instruction *> Peephole::peephole_sub(IRBuilder &builder,
                                                    Instruction *inst) {
  if (inst->get_opcode() != INST_SUB) {
    return std::nullopt;
  }
  auto sub = reinterpret_cast<ArithmeticInstruction *>(inst);
  if (sub->get_lhs()->get_id() == sub->get_rhs()->get_id()) {
    // sub a, a -> 0
    auto zero = builder.make_int_constant(0);
    auto next = builder.replace(sub, zero);
    return next;
  } else if (is_const_eq(sub->get_rhs(), 0)) {
    // sub a, 0 -> a
    builder.move_users(sub, sub->get_lhs());
    auto next = builder.rm_instruction(sub);
    return next;
  }
  return std::nullopt;
}

std::optional<Instruction *> Peephole::peephole_shr(IRBuilder &builder,
                                                    Instruction *inst) {
  if (inst->get_opcode() != INST_SHR) {
    return std::nullopt;
  }
  auto shift = dynamic_cast<BitShift *>(inst);
  if (!is_const(shift->get_rhs())) {
    return std::nullopt;
  }
  auto user_shift_it = std::find_if(
      shift->users_begin(), shift->users_end(),
      [inst](const Instruction *user) {
        if (user->get_opcode() == INST_SHR) {
          auto user_shift = dynamic_cast<const BitShift *>(user);
          return user_shift->get_lhs()->get_id() == inst->get_id() &&
                 is_const(user_shift->get_rhs());
        }
        return false;
      });
  if (user_shift_it == shift->users_end()) {
    return std::nullopt;
  }
  // v1 = shr v0, x
  // v2 = shr v1, y -> v2 = shr v0, (x+y)
  auto user_shift = dynamic_cast<BitShift *>(*user_shift_it);
  uint64_t first = get_const_value(shift->get_rhs());
  uint64_t second = get_const_value(user_shift->get_rhs());
  uint64_t result = (first + second) % (sizeof(uint64_t) * 8);
  auto result_const = builder.make_int_constant(result);
  auto result_shift = builder.make_shr(shift->get_lhs(), result_const);
  builder.replace(user_shift, result_shift);
  builder.insert_before(result_const, result_shift);
  return inst;
}

std::optional<Instruction *> Peephole::peephole_div(IRBuilder &builder,
                                                    Instruction *inst) {
  if (inst->get_opcode() != INST_DIV) {
    return std::nullopt;
  }
  auto div_inst = dynamic_cast<ArithmeticInstruction *>(inst);
  if (div_inst->get_rhs()->get_opcode() != INST_CONST) {
    return std::nullopt;
  }
  auto denominator = get_const_value(div_inst->get_rhs());
  if (denominator < 0 || (denominator & 1)) {
    return std::nullopt;
  }
  auto is_pow2 = [](uint64_t num) {
    return num != 0 && (num ^ ((~num + 1) & num)) == 0;
  };
  if (!is_pow2(static_cast<uint64_t>(denominator))) {
    return std::nullopt;
  }
  size_t power = 0;
  while (denominator > 1) {
    power++;
    denominator = denominator >> 1;
  }
  auto shift_val = builder.make_int_constant(power);
  builder.insert_before(shift_val, div_inst);
  auto shr_inst = builder.make_shr(div_inst->get_lhs(), shift_val);
  return builder.replace(div_inst, shr_inst);
}

bool Peephole::is_const_eq(Instruction *inst, int64_t value) {
  if (inst->get_opcode() != INST_CONST || inst->get_type() != INTEGER) {
    return false;
  }
  auto inst_value =
      reinterpret_cast<LoadConstant<int64_t> *>(inst)->get_value();
  return inst_value == value;
}

int64_t Peephole::get_const_value(Instruction *const_inst) {
  assert(const_inst->get_opcode() == INST_CONST &&
         const_inst->get_type() == INTEGER &&
         "Only integer const value can be extracted in compile time");
  return reinterpret_cast<LoadConstant<int64_t> *>(const_inst)->get_value();
}

bool Peephole::is_const(Instruction *inst) {
  return inst->get_opcode() == INST_CONST;
}

} // namespace koda