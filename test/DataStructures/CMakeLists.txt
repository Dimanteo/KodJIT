function(add_gtest NAME SRC)
  add_executable(${NAME} ${SRC})
  target_link_libraries(${NAME} GTest::gtest_main)
  gtest_discover_tests(${NAME})
endfunction()

add_gtest(list_test List_test.cpp)
add_gtest(graph_test Graph_test.cpp)
