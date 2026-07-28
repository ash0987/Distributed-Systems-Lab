add_executable(log_generator main.cpp log_generator.cpp)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/templates.txt ${CMAKE_CURRENT_BINARY_DIR}/templates.txt COPYONLY)

add_executable(ingest_main ingest_main.cpp)
target_link_libraries(ingest_main PRIVATE ingest core)

find_package(zstd REQUIRED)
target_link_libraries(ingest_main PRIVATE zstd::libzstd_shared)

add_executable(index_main index_main.cpp)
target_link_libraries(index_main PRIVATE ingest index)