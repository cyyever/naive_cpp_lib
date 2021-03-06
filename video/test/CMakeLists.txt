find_package(doctest REQUIRED)

set(test_progs reader_test writer_test packet_reader_test)

set(TEST_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/test_images)
set(TEST_VIDEO_DIR ${CMAKE_CURRENT_LIST_DIR}/test_video)
foreach(test_prog ${test_progs})
  add_executable(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  target_link_libraries(${test_prog} PRIVATE CyyNaiveLib::video)
  target_link_libraries(${test_prog} PRIVATE CyyNaiveLib::cv)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  target_compile_definitions(${test_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)
  target_compile_definitions(${test_prog}
                             PRIVATE IN_IMAGE=${TEST_IMAGE_DIR}/1.jpg)
  target_compile_definitions(${test_prog}
                             PRIVATE IN_URL=${TEST_VIDEO_DIR}/output.mp4)
  add_test_with_runtime_analysis(TARGET ${test_prog})
endforeach()
