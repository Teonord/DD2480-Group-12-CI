#define CATCH_CONFIG_MAIN

#include "../../external/catch.hpp"

int compilableFunction(int a, int b) {
    return a + b;
}

TEST_CASE("sampleTest2", "[abc]") {
    REQUIRE(compilableFunction(9, 10) != 21);
}

TEST_CASE("sampleTest", "[abc]") {
    REQUIRE(compilableFunction(9, 10) == 19);
}