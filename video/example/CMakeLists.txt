if(NOT WIN32)
  add_compile_options("-g")
  add_compile_options("-fsanitize=leak")
  # add_compile_options("-fsanitize=undefined")
  # add_compile_options("-fsanitize=address")
endif()


# find_package(DeepirVideo REQUIRED)
# find_package(DeepirImage REQUIRED)
# find_package(OpenCV REQUIRED COMPONENTS imgproc highgui)

set(example_progs ffmpeg_reader_example ffmpeg_writer_example ffmpeg_example)

foreach(example_prog ${example_progs})
  add_executable(${example_prog}
                 ${CMAKE_CURRENT_SOURCE_DIR}/${example_prog}.cpp)
  target_link_libraries(${example_prog} PRIVATE CyyNaiveLib::video CyyNaiveLib::cv )
endforeach()
