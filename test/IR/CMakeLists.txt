add_executable(irbuilder_test IRBuilder_test.cpp)

target_link_libraries(irbuilder_test koda::IR GTest::gtest_main)

gtest_discover_tests(irbuilder_test)
