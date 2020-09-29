CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

PROJECT(DeepirVideoExample)

INCLUDE(../cmake/all.cmake)

IF (NOT WIN32)
  add_compile_options("-g")
  add_compile_options("-fsanitize=leak")
  #  add_compile_options("-fsanitize=undefined")
#  add_compile_options("-fsanitize=address")
ENDIF()

IF (WIN32)
  SET(libavcodec_INCLUDE_DIRS "C:/ffmpeg-3.4-win64-dev/include")
  INCLUDE_DIRECTORIES(${libavcodec_INCLUDE_DIRS})
ENDIF()

FIND_PACKAGE(DeepirVideo REQUIRED)
FIND_PACKAGE(DeepirImage REQUIRED)
FIND_PACKAGE(OpenCV REQUIRED COMPONENTS imgproc highgui)

SET(example_progs ffmpeg_reader_example ffmpeg_writer_example ffmpeg_example)

FOREACH(example_prog ${example_progs})
  ADD_EXECUTABLE(${example_prog} ${CMAKE_CURRENT_SOURCE_DIR}/${example_prog}.cpp)
  TARGET_LINK_LIBRARIES(${example_prog} PRIVATE DeepirVideo ${OpenCV_LIBRARIES} DeepirImage)
ENDFOREACH()
