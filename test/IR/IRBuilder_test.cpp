#include <IR/BasicBlock.hpp>
#include <IR/IRBuilder.hpp>
#include <IR/ProgramGraph.hpp>

#include <gtest/gtest.h>

namespace koda {

TEST(IRTests, empty_prog_test) {
    ProgramGraph graph;
}

} // namespace koda