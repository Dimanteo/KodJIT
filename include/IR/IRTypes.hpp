#pragma once

#include <cstddef>

namespace koda {

using bbid_t = long long int;
using instid_t = size_t;

enum InstOpcode : unsigned {
#define INAME_DEF(name, dummy) INST_##name,
#include "IRInstEnum.def"
#undef INAME_DEF
};

inline constexpr bool is_terminator_opcode(InstOpcode opc) {
  return opc == INST_BRANCH || opc == INST_COND_BR;
}

inline constexpr const char *inst_opc_to_str(InstOpcode opc) {
  switch (opc) {
#define INAME_DEF(name, str)                                                                                 \
  case INST_##name:                                                                                          \
    return #str;
#include "IRInstEnum.def"
#undef INAME_DEF

  default:
    return "Uknown_inst";
  }
  return "Uknown_inst";
}

enum CmpFlag {
  CMP_INVALID = 0,
  CMP_EQ,
  CMP_NE,
  CMP_L,
  CMP_LE,
  CMP_G,
  CMP_GE,
};

inline constexpr const char *flag_to_str(CmpFlag flag) {
  switch (flag) {
  case CMP_INVALID:
    return "Invalid";
  case CMP_EQ:
    return "eq";
  case CMP_NE:
    return "ne";
  case CMP_L:
    return "lt";
  case CMP_LE:
    return "le";
  case CMP_G:
    return "gt";
  case CMP_GE:
    return "ge";
  default:
    return "Unknown_flag";
  }
  return "Unknown_flag";
}

} // namespace koda
