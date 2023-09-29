#pragma once

#include "stdint.h"

namespace koda {

enum InstOpcode : uint8_t { OPC_INVALID };

enum ISATypes { TYPE_INVALID = 0, TYPE_NUM, TYPE_STRING, TYPE_ARRAY };

}; // namespace koda