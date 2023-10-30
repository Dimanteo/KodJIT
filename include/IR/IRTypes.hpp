#pragma once

#include <array>
#include <cstddef>
#include <stdint.h>

namespace koda {

using bbid_t = size_t;
using instid_t = size_t;

enum InstOpcode : uint8_t {
  INST_INVALID,
  INST_ADD,
  INST_SUB,
  INST_MUL,
  INST_DIV,
  INST_MOD,
  INST_BRANCH,
  INST_COND_BR,
  INST_PHI,
  INST_PARAM,
  INST_CONST
};

inline constexpr bool isTerminatorOpcode(InstOpcode opc) {
  return opc == INST_BRANCH || opc == INST_COND_BR;
}

enum CmpFlag {
  CMP_INVALID,
  CMP_EQ,
  CMP_NE,
  CMP_L,
  CMP_LE,
  CMP_G,
  CMP_GE,
};

} // namespace koda
