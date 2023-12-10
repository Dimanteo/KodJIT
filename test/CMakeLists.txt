function(add_gtest NAME SRC)
  add_executable(${NAME} ${SRC})
  target_link_libraries(${NAME} GTest::gtest_main)
  gtest_discover_tests(${NAME})
endfunction()

add_subdirectory(DataStructures)
add_subdirectory(IR)