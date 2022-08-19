#include <catch2/catch_test_macros.hpp>

#include <sc/format.hpp>

namespace sc {
    TEST_CASE("fast_format_no_name_no_id", "[sc::format]") {
        detail::fast_format_spec spec{"", 0};
        REQUIRE(spec.has_name == false);
        REQUIRE(spec.has_id == false);
        REQUIRE(spec.size == 2);
    }

    TEST_CASE("fast_format_with_name", "[sc::format]") {
        detail::fast_format_spec spec{"olivia", 0};
        REQUIRE(spec.has_name == true);
        REQUIRE(spec.has_id == false);
        REQUIRE("olivia" == spec.name);
        REQUIRE(spec.size == 8);
    }

    TEST_CASE("fast_format_with_id", "[sc::format]") {
        detail::fast_format_spec spec{"42", 0};
        REQUIRE(spec.has_name == false);
        REQUIRE(spec.has_id == true);
        REQUIRE(42 == spec.id);
        REQUIRE(spec.size == 4);
    }

    TEST_CASE("fast_format_with_id_2", "[sc::format]") {
        detail::fast_format_spec spec{"5", 0};
        REQUIRE(spec.has_name == false);
        REQUIRE(spec.has_id == true);
        REQUIRE(5 == spec.id);
        REQUIRE(spec.size == 3);
    }

    TEST_CASE("fast_format_with_no_name_no_id_colon", "[sc::format]") {
        detail::fast_format_spec spec{":d", 0};
        REQUIRE(spec.has_name == false);
        REQUIRE(spec.has_id == false);
        REQUIRE(spec.type == 'd');
        REQUIRE(spec.size == 4);
    }

    TEST_CASE("fast_format_zero_pad_width", "[sc::format]") {
        detail::fast_format_spec spec{":08x", 0};
        REQUIRE(spec.zero_pad == true);
        REQUIRE(spec.padding_width == 8);
        REQUIRE(spec.type == 'x');
        REQUIRE(spec.size == 6);
    }

    TEST_CASE("repl_fields_empty", "[sc::format]") {
        repl_fields fields{""};

        auto i = fields.begin();
        REQUIRE(i == fields.end());
    }

    TEST_CASE("repl_fields_one_empty", "[sc::format]") {
        repl_fields fields{"{}"};

        auto i = fields.begin();
        REQUIRE(*i == "");
        REQUIRE(i != fields.end());

        ++i;
        REQUIRE(i == fields.end());
    }

    TEST_CASE("repl_fields_one_has_something", "[sc::format]") {
        repl_fields fields{"Hello {name}!"};

        auto i = fields.begin();
        REQUIRE(*i == "name");
        REQUIRE(i != fields.end());

        ++i;
        REQUIRE(i == fields.end());
    }

    TEST_CASE("repl_fields_two_have_something", "[sc::format]") {
        repl_fields fields{"Hi {name1} and {name2}!"};

        auto i = fields.begin();
        REQUIRE(*i == "name1");
        REQUIRE(i != fields.end());

        ++i;
        REQUIRE(*i == "name2");
        REQUIRE(i != fields.end());

        ++i;
        REQUIRE(i == fields.end());
    }

    TEST_CASE("repl_fields_for_each", "[sc::format]") {
        repl_fields fields{"Hi {name1}, {name2}, and {}!"};

        auto count = 0;
        for (auto f : fields) {
            count++;
        }

        REQUIRE(count == 3);
    }

    TEST_CASE("string_repl", "[sc::format]") {
        REQUIRE(format("Hello, {:s} is a good day!"_sc, "today"_sc) == "Hello, today is a good day!"_sc);
    }

    TEST_CASE("string_repl2", "[sc::format]") {
        REQUIRE(format("This {:s} is a {:s}."_sc, "box"_sc, "package"_sc) == "This box is a package."_sc);
    }

    TEST_CASE("string_repl_legacy", "[sc::format]") {
        REQUIRE(format("{} {} {} arguments."_sc, "There"_sc, "are"_sc, "three"_sc) == "There are three arguments."_sc);
        REQUIRE(format("{}"_sc, "five"_sc) == "five"_sc);
        REQUIRE(format("{}"_sc, "0"_sc) == "0"_sc);
        REQUIRE(format("-{}"_sc, "0"_sc) == "-0"_sc);
        REQUIRE(format("{}-"_sc, "0"_sc) == "0-"_sc);
        REQUIRE(format("{}"_sc, ""_sc) == ""_sc);
        REQUIRE(format("[{}"_sc, ""_sc) == "["_sc);
        REQUIRE(format("{}]"_sc, ""_sc) == "]"_sc);
        REQUIRE(format("[{}]"_sc, ""_sc) == "[]"_sc);
    }

    enum class Cmd {
        READ,
        WRITE,
        NOOP
    };

    TEST_CASE("EnumToString", "[sc::format]") {
        REQUIRE(sc::detail::enum_as_string<Cmd::READ>() == std::string_view{"READ"});
    }

    TEST_CASE("enums", "[sc::format]") {
        REQUIRE(format("Command = {}"_sc, enum_<Cmd::WRITE>) == "Command = WRITE"_sc);
        REQUIRE(format("Command = {}"_sc, enum_<Cmd::READ>) == "Command = READ"_sc);
    }

    struct ExampleTypeName {};

    TEST_CASE("typenames", "[sc::format]") {
        REQUIRE(format("Type = {}"_sc, type_<ExampleTypeName>).str == "Type = sc::ExampleTypeName"_sc);
        REQUIRE(format("Type = {}"_sc, type_name{ExampleTypeName{}}) == "Type = sc::ExampleTypeName"_sc);
    }

    static_assert(sc::to_string_constant(int_<5>) == "5"_sc);
    static_assert(sc::to_string_constant(int_<0>) == "0"_sc);
    static_assert(sc::to_string_constant(int_<10>) == "10"_sc);
    static_assert(sc::to_string_constant(int_<-1>) == "-1"_sc);
    static_assert(sc::to_string_constant(int_<1>) == "1"_sc);
    static_assert(sc::to_string_constant(int_<-6345>) == "-6345"_sc);
    static_assert(sc::to_string_constant(int_<2147483647>) == "2147483647"_sc);
    //static_assert(sc::to_string_constant(int_<-2147483648>) == "-2147483648"_sc);
    static_assert(sc::to_string_constant(int_<-2147483647>) == "-2147483647"_sc);
    static_assert(sc::to_string_constant(int_<4>, int_<2>) == "100"_sc);
    static_assert(sc::to_string_constant(int_<10>, int_<2>) == "1010"_sc);
    static_assert(sc::to_string_constant(int_<8>, int_<8>) == "10"_sc);
    static_assert(sc::to_string_constant(int_<16>, int_<8>) == "20"_sc);
    static_assert(sc::to_string_constant(int_<16>, int_<16>) == "10"_sc);
    static_assert(sc::to_string_constant(int_<10>, int_<16>) == "a"_sc);
    static_assert(sc::to_string_constant(int_<0xca7b007>, int_<16>) == "ca7b007"_sc);
    static_assert(sc::to_string_constant(int_<0x101d0115>, int_<16>) == "101d0115"_sc);

    template<typename IntegralT, IntegralT ValueT>
    struct my_integral_constant : public std::integral_constant<IntegralT, ValueT> {};

    TEST_CASE("unformatted_integral_constant", "[sc::format]") {
        REQUIRE(format("The answer is {}."_sc, int_<42>) == "The answer is 42."_sc);
        REQUIRE(format("{}"_sc, my_integral_constant<int, 42>{}) == "42"_sc);
    }

    TEST_CASE("mixed_types", "[sc::format]") {
        REQUIRE(format("Only {} more days until {}."_sc, int_<10000>, "retirement"_sc) == "Only 10000 more days until retirement."_sc);
    }

    TEST_CASE("int_format_options", "[sc::format]") {
        REQUIRE(format("{:d}"_sc, int_<42>) == "42"_sc);
        REQUIRE(format("{:b}"_sc, int_<17>) == "10001"_sc);
        REQUIRE(format("{:x}"_sc, int_<0xba115>) == "ba115"_sc);
        REQUIRE(format("{:X}"_sc, int_<0xba115>) == "BA115"_sc);
        REQUIRE(format("{:o}"_sc, int_<16>) == "20"_sc);
        REQUIRE(format("{:08x}"_sc, int_<0xbea75>) == "000bea75"_sc);
        REQUIRE(format("{:8x}"_sc, int_<0xbea75>) == "   bea75"_sc);
        REQUIRE(format("{:4x}"_sc, int_<0xbea75>) == "bea75"_sc);
        REQUIRE(format("{:04x}"_sc, int_<0xbea75>) == "bea75"_sc);
    }

    TEST_CASE("lazy_runtime_values", "[sc::format]") {
        REQUIRE(format("{}"_sc, 0) == (lazy_string_format{"{}"_sc, cib::make_tuple(0)}));
        REQUIRE(format("{}"_sc, 1) == (lazy_string_format{"{}"_sc, cib::make_tuple(1)}));
        REQUIRE(format("I am {} and my sister is {}"_sc, 6, 8) == (lazy_string_format{"I am {} and my sister is {}"_sc, cib::make_tuple(6, 8)}));
        REQUIRE(format("{}"_sc, 100) != (lazy_string_format{"{}"_sc, cib::make_tuple(99)}));
        REQUIRE(format("{}"_sc, true) == (lazy_string_format{"{}"_sc, cib::make_tuple(true)}));
    }

    TEST_CASE("mixed_runtime_compile_time", "[sc::format]") {
        REQUIRE(
            format("My name is {} and I am {} years old"_sc, "Olivia"_sc, 8) ==
            (lazy_string_format{"My name is Olivia and I am {} years old"_sc, cib::make_tuple(8)}));
    }

    TEST_CASE("format_a_formatted_string", "[sc::format]") {
        REQUIRE(
            format("Hello {}!"_sc,
               format("Abigail"_sc)
            )
            ==
            "Hello Abigail!"_sc
        );

        REQUIRE(
            format("The value is {}."_sc,
               format("(year={})"_sc, 2022)
            )
            ==
            (lazy_string_format{
                "The value is (year={})."_sc,
                cib::make_tuple(2022)
            })
        );

        REQUIRE(
            format("a{}b{}c"_sc,
                format("1{}2{}3"_sc, 10, 20),
                format("4{}5{}6"_sc, 30, 40)
            )
            ==
            (lazy_string_format{
                "a1{}2{}3b4{}5{}6c"_sc,
                cib::make_tuple(10, 20, 30, 40)
            })
        );
    }


//

//
//    // ignore escaped curly braces
//    //static_assert(format("Ignore {{ and }}."_sc) == "Ignore {{ and }}."_sc);
//

//    // use order arg ids to pick which argument to use for each replacement field
//    static_assert(format("{0} {1}"_sc, "bumbly"_sc, "wumbly"_sc) == "bumbly wumbly"_sc);
//    static_assert(format("{1} {0}"_sc, "bumbly"_sc, "wumbly"_sc) == "wumbly bumbly"_sc);
//
//
//    // use named arg ids to pick which argument to use for each replacement field
//    static_assert(
//        format(
//            "My name is {name}, I am {age} years old."_sc,
//            arg("name"_sc, "Luke"_sc),
//            arg("age"_sc, int_<36>))
//        ==
//        "My name is Luke, I am 36 years old."_sc);

    // TODO: add tests for mismatched format specifiers and arguments
}