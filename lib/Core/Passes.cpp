#include <Core/Analysis.hpp>
#include <Core/Compiler.h>
#include <Core/Passes.hpp>
#include <DataStructures/Graph.hpp>
#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>

namespace koda {

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
      auto folded_inst =
          compiler.graph().create_instruction<LoadConstant<int64_t>>(INTEGER,
                                                                      result);
      inst = builder.replace(inst, folded_inst);
    }
  }
  // Remove redundant instructions
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

} // namespace koda