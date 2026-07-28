add_library(ingest segment_writer.cpp segment_reader.cpp)
target_link_libraries(ingest PUBLIC core)
target_include_directories(ingest PUBLIC ${CMAKE_SOURCE_DIR})

find_package(zstd REQUIRED)
target_link_libraries(ingest PUBLIC zstd::libzstd_shared)