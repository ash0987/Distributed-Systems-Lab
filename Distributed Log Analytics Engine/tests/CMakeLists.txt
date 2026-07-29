add_executable(test_smoke test_smoke.cpp)
add_test(NAME test_smoke COMMAND test_smoke)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)
target_link_libraries(test_smoke PRIVATE Catch2::Catch2WithMain)

add_executable(test_log_record test_log_record.cpp)
target_link_libraries(test_log_record PRIVATE core)

add_executable(wal_tests wal_tests.cpp)
target_include_directories(wal_tests PRIVATE ${CMAKE_SOURCE_DIR})

add_executable(segment_test segment_test.cpp)
target_link_libraries(segment_test PRIVATE ingest)

add_executable(segment_reader_test segment_reader_test.cpp)
target_link_libraries(segment_reader_test PRIVATE ingest)

add_executable(tokenizer_test test_tokenizer.cpp)
target_link_libraries(tokenizer_test PRIVATE index)

add_executable(inverted_index_test test_inverted_index.cpp)
target_link_libraries(inverted_index_test PRIVATE index)

add_executable(test_query_ast test_query_ast.cpp)
target_link_libraries(test_query_ast PRIVATE index)

add_executable(test_query_parser test_query_parser.cpp)
target_link_libraries(test_query_parser PRIVATE index)

add_executable(test_query_execution test_query_execution.cpp)
target_link_libraries(test_query_execution PRIVATE index)

add_executable(test_query_benchmark test_query_benchmark.cpp)
target_link_libraries(test_query_benchmark PRIVATE index)