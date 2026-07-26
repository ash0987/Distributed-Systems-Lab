add_executable(log_generator main.cpp log_generator.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/templates.txt ${CMAKE_CURRENT_BINARY_DIR}/templates.txt COPYONLY)


