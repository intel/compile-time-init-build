#include <seq/builder.hpp>
#include <seq/impl.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
int attempt_count;
std::string result;
} // namespace

TEST_CASE("build and run empty seq", "[seq]") {
    seq::builder<> builder;
    auto seq_impl = builder.topo_sort<seq::impl, 0>();
    CHECK(seq_impl.forward() == seq::status::DONE);
    CHECK(seq_impl.backward() == seq::status::DONE);
}

TEST_CASE("build seq with one step and run forwards and backwards", "[seq]") {
    result = "";
    seq::builder<> builder;

    auto s = seq::step(
        "S"_sc,
        []() -> seq::status {
            result += "F";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B";
            return seq::status::DONE;
        });

    builder.add(s);

    auto seq_impl = builder.topo_sort<seq::impl, 1>();

    CHECK(seq_impl.forward() == seq::status::DONE);
    CHECK(result == "F");

    CHECK(seq_impl.backward() == seq::status::DONE);
    CHECK(result == "FB");
}

TEST_CASE("build seq with a forward step that takes a while to finish",
          "[seq]") {
    result = "";
    attempt_count = 0;
    seq::builder<> builder;

    auto s = seq::step(
        "S"_sc,
        []() -> seq::status {
            if (attempt_count++ < 3) {
                result += "F";
                return seq::status::NOT_DONE;
            } else {
                return seq::status::DONE;
            }
        },
        []() -> seq::status {
            result += "B";
            return seq::status::DONE;
        });

    builder.add(s);

    auto seq_impl = builder.topo_sort<seq::impl, 1>();

    SECTION("forward can be called") {
        CHECK(seq_impl.forward() == seq::status::NOT_DONE);
        CHECK(result == "F");
        CHECK(seq_impl.forward() == seq::status::NOT_DONE);
        CHECK(result == "FF");
        CHECK(seq_impl.forward() == seq::status::NOT_DONE);
        CHECK(result == "FFF");
        CHECK(seq_impl.forward() == seq::status::DONE);
        CHECK(result == "FFF");
    }
    SECTION(
        "backward can be called, but will not proceed until forward is done") {
        CHECK(seq_impl.forward() == seq::status::NOT_DONE);
        CHECK(result == "F");
        CHECK(seq_impl.backward() == seq::status::NOT_DONE);
        CHECK(result == "FF");
        CHECK(seq_impl.backward() == seq::status::NOT_DONE);
        CHECK(result == "FFF");
        CHECK(seq_impl.backward() == seq::status::DONE);
        CHECK(result == "FFFB");
    }
}

TEST_CASE("build seq with a backward step that takes a while to finish",
          "[seq]") {
    result = "";
    attempt_count = 0;
    seq::builder<> builder;

    auto s = seq::step(
        "S"_sc,
        []() -> seq::status {
            result += "F";
            return seq::status::DONE;
        },
        []() -> seq::status {
            if (attempt_count++ < 3) {
                result += "B";
                return seq::status::NOT_DONE;
            } else {
                return seq::status::DONE;
            }
        });

    builder.add(s);

    auto seq_impl = builder.topo_sort<seq::impl, 1>();

    SECTION("backward can be called") {
        CHECK(seq_impl.forward() == seq::status::DONE);
        CHECK(result == "F");
        CHECK(seq_impl.backward() == seq::status::NOT_DONE);
        CHECK(result == "FB");
        CHECK(seq_impl.backward() == seq::status::NOT_DONE);
        CHECK(result == "FBB");
        CHECK(seq_impl.backward() == seq::status::NOT_DONE);
        CHECK(result == "FBBB");
        CHECK(seq_impl.backward() == seq::status::DONE);
        CHECK(result == "FBBB");
    }
    SECTION(
        "forward can be called, but will not proceed until backward is done") {
        CHECK(seq_impl.forward() == seq::status::DONE);
        CHECK(result == "F");
        CHECK(seq_impl.backward() == seq::status::NOT_DONE);
        CHECK(result == "FB");
        CHECK(seq_impl.forward() == seq::status::NOT_DONE);
        CHECK(result == "FBB");
        CHECK(seq_impl.forward() == seq::status::NOT_DONE);
        CHECK(result == "FBBB");
        CHECK(seq_impl.forward() == seq::status::DONE);
        CHECK(result == "FBBBF");
    }
}

TEST_CASE("build seq with three steps and run forwards and backwards",
          "[seq]") {
    result = "";
    seq::builder<> builder;

    auto s1 = seq::step(
        "S1"_sc,
        []() -> seq::status {
            result += "F1";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B1";
            return seq::status::DONE;
        });

    auto s2 = seq::step(
        "S2"_sc,
        []() -> seq::status {
            result += "F2";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B2";
            return seq::status::DONE;
        });

    auto s3 = seq::step(
        "S3"_sc,
        []() -> seq::status {
            result += "F3";
            return seq::status::DONE;
        },
        []() -> seq::status {
            result += "B3";
            return seq::status::DONE;
        });

    builder.add(s1 >> s2 >> s3);

    auto seq_impl = builder.topo_sort<seq::impl, 3>();

    CHECK(seq_impl.forward() == seq::status::DONE);
    CHECK(result == "F1F2F3");
    CHECK(seq_impl.backward() == seq::status::DONE);
    CHECK(result == "F1F2F3B3B2B1");
}
