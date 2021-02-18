find_package(OpenCV REQUIRED COMPONENTS imgproc highgui)

set(example_progs ffmpeg_reader_example ffmpeg_writer_example ffmpeg_example)

foreach(example_prog ${example_progs})
  add_executable(${example_prog}
                 ${CMAKE_CURRENT_SOURCE_DIR}/${example_prog}.cpp)
  target_link_libraries(
    ${example_prog} PRIVATE CyyNaiveLib::video CyyNaiveLib::cv
                            ${OpenCV_LIBRARIES})
endforeach()
