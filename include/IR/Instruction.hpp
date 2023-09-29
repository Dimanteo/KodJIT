#pragma once

#include <IR/IRCommon.h>
#include <ISA/constants.hpp>
#include <DataStructures/List.hpp>

#include <stddef.h>
#include <vector>

namespace koda {

class BasicBlock;

// Base class for IR instruction
//
class Instruction : public IntrusiveListNode {
public:
  using InstPtr = Instruction *;
  using InstType = ISATypes;

private:
  instid_t m_id;
  InstOpcode m_opcode = OPC_INVALID;
  InstType m_type = TYPE_INVALID;
  BasicBlock *m_bblock;
  std::vector<InstPtr> users;

public:
  Instruction() = default;

  Instruction(InstOpcode opc, BasicBlock bb);

  virtual ~Instruction() = default;

  InstOpcode getOpcode() const;

  InstType getType() const;

  const std::vector<InstPtr> &users() const;
};

}; // namespace koda