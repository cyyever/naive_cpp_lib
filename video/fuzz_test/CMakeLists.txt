CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

INCLUDE(${CMAKE_CURRENT_LIST_DIR}/../cmake/fuzzing.cmake)
FIND_PACKAGE(DeepirImage REQUIRED)

SET(fuzzing_progs reader_fuzzing writer_fuzzing)

FOREACH(fuzzing_prog ${fuzzing_progs})
  ADD_EXECUTABLE(${fuzzing_prog} ${CMAKE_CURRENT_LIST_DIR}/${fuzzing_prog}.cpp)
  TARGET_LINK_LIBRARIES(${fuzzing_prog} PRIVATE DeepirVideo)
  TARGET_LINK_LIBRARIES(${fuzzing_prog} PRIVATE DeepirImage)
  set_target_properties(${fuzzing_prog} PROPERTIES ENVIRONMENT TEST_NO_CUDA=1)
  add_fuzzing(TARGET ${fuzzing_prog})
ENDFOREACH()
