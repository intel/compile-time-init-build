#include <catch2/catch_test_macros.hpp>

#include <cib/cib.hpp>

#include <string>
#include <seq/impl.hpp>
#include <seq/builder.hpp>

namespace {

    TEST_CASE("construct empty sequencer", "[seq]") {
        seq::impl<> seq_impl{};

        SECTION("forward can be called without issue") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
        }

        SECTION("backward can be called without issue") {
            REQUIRE(seq_impl.backward() == seq::status::DONE);
        }
    }

    std::string result;

    TEST_CASE("construct sequencer with a single step", "[seq]") {
        result = "";

        seq::func_ptr forward_steps[] = {
            []() -> seq::status {
                result += "F0";
                return seq::status::DONE;
            }
        };

        seq::func_ptr backward_steps[] = {
            []() -> seq::status {
                result += "B0";
                return seq::status::DONE;
            }
        };

        seq::impl<1> seq_impl{forward_steps, backward_steps};


        SECTION("forward can be called") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0");

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0");
        }

        SECTION("backward can be called") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0B0");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0B0");
        }

        SECTION("forward and backwards can be called multiple times") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0B0");

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0B0F0");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0B0F0B0");
        }
    }


    TEST_CASE("construct sequencer with multiple steps", "[seq]") {
        result = "";

        seq::func_ptr forward_steps[] = {
            []() -> seq::status {
                result += "F0";
                return seq::status::DONE;
            },

            []() -> seq::status {
                result += "F1";
                return seq::status::DONE;
            },

            []() -> seq::status {
                result += "F2";
                return seq::status::DONE;
            }
        };

        seq::func_ptr backward_steps[] = {
            []() -> seq::status {
                result += "B0";
                return seq::status::DONE;
            },

            []() -> seq::status {
                result += "B1";
                return seq::status::DONE;
            },

            []() -> seq::status {
                result += "B2";
                return seq::status::DONE;
            }
        };

        seq::impl<3> seq_impl{forward_steps, backward_steps};


        SECTION("forward can be called") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2");

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2");
        }

        SECTION("backward can be called") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2B2B1B0");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2B2B1B0");
        }

        SECTION("forward and backwards can be called multiple times") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2B2B1B0");

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2B2B1B0F0F1F2");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2B2B1B0F0F1F2B2B1B0");
        }
    }

    int attempt_count;

    TEST_CASE("construct sequencer with a single step that never finishes", "[seq]") {
        result = "";
        attempt_count = 0;

        seq::func_ptr forward_steps[] = {
            []() -> seq::status {

                if (attempt_count < 3) {
                    result += "F0";
                    attempt_count++;
                    return seq::status::NOT_DONE;
                } else {
                    return seq::status::DONE;
                }

            }
        };

        seq::func_ptr backward_steps[] = {
            []() -> seq::status {
                result += "B0";
                return seq::status::DONE;
            }
        };

        seq::impl<1> seq_impl{forward_steps, backward_steps};


        SECTION("forward can be called") {
            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0");

            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0F0");

            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0F0F0");

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F0F0");
        }

        SECTION("backward can be called, but will not proceed") {
            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0");

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0F0");

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0F0F0");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0F0F0B0");
        }
    }

    TEST_CASE("construct sequencer with a single backward step that never finishes", "[seq]") {
        result = "";
        attempt_count = 0;

        seq::func_ptr forward_steps[] = {
                []() -> seq::status {
                    result += "F0";
                    return seq::status::DONE;

                }
        };

        seq::func_ptr backward_steps[] = {
                []() -> seq::status {
                    if (attempt_count < 3) {
                        result += "B0";
                        attempt_count++;
                        return seq::status::NOT_DONE;
                    } else {
                        return seq::status::DONE;
                    }

                }
        };

        seq::impl<1> seq_impl{forward_steps, backward_steps};


        SECTION("backward can be called") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0");

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0B0");

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0B0B0");

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0B0B0B0");

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0B0B0B0");
        }

        SECTION("forward can be called, but will not proceed") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0");

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0B0");

            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0B0B0");

            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0B0B0B0");

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0B0B0B0F0");
        }
    }

    seq::status f0_step_status = seq::status::DONE;
    seq::status f1_step_status = seq::status::DONE;
    seq::status f2_step_status = seq::status::DONE;
    seq::status b0_step_status = seq::status::DONE;
    seq::status b1_step_status = seq::status::DONE;
    seq::status b2_step_status = seq::status::DONE;

    TEST_CASE("run sequencer with multiple steps", "[seq]") {
        result = "";
        f0_step_status = seq::status::DONE;
        f1_step_status = seq::status::DONE;
        f2_step_status = seq::status::DONE;
        b0_step_status = seq::status::DONE;
        b1_step_status = seq::status::DONE;
        b2_step_status = seq::status::DONE;

        seq::func_ptr forward_steps[] = {
            []() -> seq::status {
                result += "F0";
                return f0_step_status;
            },

            []() -> seq::status {
                result += "F1";
                return f1_step_status;
            },

            []() -> seq::status {
                result += "F2";
                return f2_step_status;
            }
        };

        seq::func_ptr backward_steps[] = {
            []() -> seq::status {
                result += "B0";
                return b0_step_status;
            },

            []() -> seq::status {
                result += "B1";
                return b1_step_status;
            },

            []() -> seq::status {
                result += "B2";
                return b2_step_status;
            }
        };

        seq::impl<3> seq_impl{forward_steps, backward_steps};

        SECTION("backward can be called after second forward step") {
            f1_step_status = seq::status::NOT_DONE;
            REQUIRE(seq_impl.forward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0F1");

            f1_step_status = seq::status::DONE;

            REQUIRE(seq_impl.backward() == seq::status::DONE);
            REQUIRE(result == "F0F1F1B1B0");
        }

        SECTION("forward can be called after second backward step") {
            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2");

            b1_step_status = seq::status::NOT_DONE;

            REQUIRE(seq_impl.backward() == seq::status::NOT_DONE);
            REQUIRE(result == "F0F1F2B2B1");

            b1_step_status = seq::status::DONE;

            REQUIRE(seq_impl.forward() == seq::status::DONE);
            REQUIRE(result == "F0F1F2B2B1B1F1F2");
        }
    }

    TEST_CASE("build and run empty seq", "[seq]") {
        seq::builder<> builder;
        auto seq_impl = builder.internal_build<0>();
        REQUIRE(seq_impl.forward() == seq::status::DONE);
    }

    TEST_CASE("build seq with one step and run forwards and backwards", "[seq]") {
        result = "";

        seq::builder<> builder;

        auto s1 = seq::step("S1"_sc,
            []() -> seq::status {
                result += "F1";
                return seq::status::DONE;
            },
            []() -> seq::status {
                result += "B1";
                return seq::status::DONE;
            }
        );

        builder.add(s1);

        auto seq_impl = builder.internal_build<1>();


        REQUIRE(seq_impl.forward() == seq::status::DONE);
        REQUIRE(result == "F1");

        REQUIRE(seq_impl.backward() == seq::status::DONE);
        REQUIRE(result == "F1B1");
    }

    TEST_CASE("build seq with three steps and run forwards and backwards", "[seq]") {
        result = "";

        seq::builder<> builder;

        auto s1 = seq::step("S1"_sc,
            []() -> seq::status {
                result += "F1";
                return seq::status::DONE;
            },
            []() -> seq::status {
                result += "B1";
                return seq::status::DONE;
            }
        );

        auto s2 = seq::step("S2"_sc,
            []() -> seq::status {
                result += "F2";
                return seq::status::DONE;
            },
            []() -> seq::status {
                result += "B2";
                return seq::status::DONE;
            }
        );

        auto s3 = seq::step("S3"_sc,
            []() -> seq::status {
                result += "F3";
                return seq::status::DONE;
            },
            []() -> seq::status {
                result += "B3";
                return seq::status::DONE;
            }
        );

        builder.add(s1 >> s2 >> s3);

        auto seq_impl = builder.internal_build<3>();


        REQUIRE(seq_impl.forward() == seq::status::DONE);
        REQUIRE(result == "F1F2F3");

        REQUIRE(seq_impl.backward() == seq::status::DONE);
        REQUIRE(result == "F1F2F3B3B2B1");
    }
}
