include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/fuzzing.cmake)

set(ENV{FUZZING_TIMEOUT} 10)
set(ENV{MAX_FUZZING_TIME} 10)
file(GLOB fuzzing_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

foreach(fuzzing_source IN LISTS fuzzing_sources)
  get_filename_component(fuzzing_prog ${fuzzing_source} NAME_WE)
  add_executable(fuzz_${fuzzing_prog}
                 ${CMAKE_CURRENT_LIST_DIR}/${fuzzing_prog}.cpp)
  target_link_libraries(fuzz_${fuzzing_prog} PRIVATE MyCXXLib::log)
  add_fuzzing(TARGET fuzz_${fuzzing_prog} UBSAN FALSE)
endforeach()
