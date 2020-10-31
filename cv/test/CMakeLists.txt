find_package(doctest REQUIRED)
find_package(Threads REQUIRED)

set(TEST_IMAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/test_images)
file(GLOB test_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/*/*.cpp)
get_target_property(def_list CyyNaiveLib::cv INTERFACE_COMPILE_DEFINITIONS)
foreach(test_source IN LISTS test_sources)
  get_filename_component(test_prog ${test_source} NAME_WE)
  add_executable(${test_prog} ${CMAKE_CURRENT_LIST_DIR}/${test_prog}.cpp)
  target_link_libraries(${test_prog} PRIVATE CyyNaiveLib::cv)
  target_link_libraries(${test_prog} PRIVATE doctest::doctest)
  target_link_libraries(${test_prog} PRIVATE Threads::Threads)
  target_compile_definitions(${test_prog}
                             PRIVATE IN_IMAGE=${TEST_IMAGE_DIR}/1.jpg)

  target_compile_definitions(${test_prog}
                             PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

  if("HAVE_GPU_MAT" IN_LIST def_list)
    target_compile_definitions(${test_prog} PRIVATE USE_GPU)
    add_test_with_runtime_analysis(
      TARGET
      ${test_prog}
      WITH_CPU_ANALYSIS
      TRUE
      CUDA-MEMCHECK
      FALSE
      CUDA-SYNCCHECK
      TRUE
      CUDA-RACECHECK
      TRUE
      TSAN
      TRUE
      HELGRIND
      TRUE)
  else()
    add_test_with_runtime_analysis(TARGET ${test_prog} WITH_GPU_ANALYSIS FALSE)
  endif()
endforeach()
