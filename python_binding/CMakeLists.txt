file(GLOB SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(cyy_naive_cpp_extension ${SOURCE})
if(BUILD_TORCH_PYTHON_BINDING)
  find_package(Torch REQUIRED)
  target_link_directories(cyy_naive_cpp_extension PUBLIC ${pytorch_path}/lib)
  target_include_directories(cyy_naive_cpp_extension
                             PUBLIC ${pytorch_path}/include)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC torch_python)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC cyy_naive_lib_torch)
  target_compile_definitions(cyy_naive_cpp_extension
                             PUBLIC BUILD_TORCH_PYTHON_BINDING)
endif()
if(BUILD_VIDEO_PYTHON_BINDING)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC cyy_naive_lib_video)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC cyy_naive_lib_cv)
  target_compile_definitions(cyy_naive_cpp_extension
                             PUBLIC BUILD_VIDEO_PYTHON_BINDING)
endif()

if(BUILD_CV_PYTHON_BINDING)
  target_link_libraries(cyy_naive_cpp_extension PUBLIC cyy_naive_lib_cv)
  target_compile_definitions(cyy_naive_cpp_extension
                             PUBLIC BUILD_CV_PYTHON_BINDING)
endif()
