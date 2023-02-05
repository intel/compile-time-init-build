#include <msg/detail/bitset.hpp>

#include <catch2/catch_test_macros.hpp>

namespace msg::detail {

TEST_CASE("equality", "[bitset]") {
    REQUIRE(bitset<32>{1, 2, 3, 4, 5} == bitset<32>{1, 2, 3, 4, 5});
    REQUIRE(bitset<32>{1, 2, 3} != bitset<32>{6});
}

TEST_CASE("add", "[bitset]") {
    auto s = bitset<64>{};

    s.add(0);
    REQUIRE(s == bitset<64>{0});

    s.add(31);
    REQUIRE(s == bitset<64>{0, 31});

    s.add(32);
    REQUIRE(s == bitset<64>{0, 31, 32});

    s.add(40);
    REQUIRE(s == bitset<64>{0, 31, 32, 40});
}

TEST_CASE("remove", "[bitset]") {
    auto s = bitset<64>{0, 31, 32, 40};

    s.remove(0);
    REQUIRE(s == bitset<64>{31, 32, 40});

    s.remove(31);
    REQUIRE(s == bitset<64>{32, 40});

    s.remove(32);
    REQUIRE(s == bitset<64>{40});

    s.remove(40);
    REQUIRE(s == bitset<64>{});
}

TEST_CASE("contains", "[bitset]") {
    auto s = bitset<64>{0, 31, 36};

    REQUIRE(s.contains(0));
    REQUIRE(!s.contains(4));
    REQUIRE(!s.contains(27));
    REQUIRE(s.contains(31));
    REQUIRE(!s.contains(32));
    REQUIRE(s.contains(36));
}

TEST_CASE("for_each", "[bitset]") {
    auto s = bitset<32>{16, 8, 1};
    auto result = bitset<32>{};

    s.for_each([&](auto i){
        result.add(i);
    });

    REQUIRE(result == s);
}

TEST_CASE("long for_each", "[bitset]") {
    auto s = bitset<64>{47, 32, 16, 8, 1};
    auto result = bitset<64>{};

    s.for_each([&](auto i){
        result.add(i);
    });

    REQUIRE(result == s);
}
}
