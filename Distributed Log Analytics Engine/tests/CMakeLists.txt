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
