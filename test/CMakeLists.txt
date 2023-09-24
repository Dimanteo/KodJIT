add_executable(list_test List_test.cpp)

target_link_libraries(list_test GTest::gtest_main intr_list)

gtest_discover_tests(list_test)
