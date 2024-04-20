add_gtest(core_test Core_test.cpp)
target_link_libraries(core_test koda::core koda::IR koda::IR::printer)
