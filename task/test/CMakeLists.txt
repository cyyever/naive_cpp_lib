include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/test.cmake)

find_package(doctest REQUIRED)

set(test_progs queue_scheduler_test)

foreach(test_prog ${test_progs})
  add_executable(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  target_link_libraries(${test_prog} PRIVATE cyy_naive_lib_task)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  target_compile_definitions(${test_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
  add_test_with_runtime_analysis(TARGET ${test_prog})
endforeach()
