add_gtest(irbuilder_test IRBuilder_test.cpp)
target_link_libraries(irbuilder_test koda::IR koda::IR::printer)

add_gtest(loop_analyzer_test LoopAnalyzer_test.cpp)
target_link_libraries(loop_analyzer_test koda::IR koda::IR::printer)