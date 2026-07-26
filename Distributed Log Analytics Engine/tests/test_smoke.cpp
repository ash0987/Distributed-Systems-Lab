#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>

TEST_CASE("sanity check") {
    REQUIRE(2 + 2 == 4);
}