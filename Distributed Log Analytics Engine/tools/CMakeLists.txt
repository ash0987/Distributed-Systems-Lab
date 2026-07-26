add_executable(log_generator main.cpp log_generator.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/templates.txt ${CMAKE_CURRENT_BINARY_DIR}/templates.txt COPYONLY)
include(FetchContent)
FetchContent_Declare(
  zstd
  GIT_REPOSITORY https://github.com/facebook/zstd.git
  GIT_TAG v1.5.6
  SOURCE_SUBDIR build/cmake
)
FetchContent_MakeAvailable(zstd)

target_link_libraries(log_generator PRIVATE libzstd_static)


