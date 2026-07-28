add_library(index query_parser.cpp query_ast.cpp tokenizer.cpp inverted_index.cpp)
target_link_libraries(index PUBLIC core)
target_include_directories(index PUBLIC ${CMAKE_SOURCE_DIR})