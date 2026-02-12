import gen_str_catalog as gen


def test_split_args():
    assert gen.split_args("1, 2, 3") == ["1", "2", "3"]


def test_split_args_templates():
    assert gen.split_args("array<char, 1>, 2, array<array<char, 3>, 3>") == [
        "array<char, 1>",
        "2",
        "array<array<char, 3>, 3>",
    ]


def test_named_arg_from_cpp_type():
    na = gen.NamedArg.from_cpp_type(
        "sc::named_arg<sc::string<(char)100, (char)101, (char)102>, 1, 2>"
    )
    assert na == gen.NamedArg("def", 1, 2)


def test_named_arg_to_cpp_type():
    na = gen.NamedArg("def", 1, 2)
    assert (
        na.to_cpp_type()
        == "sc::named_arg<sc::string<static_cast<char>(100), static_cast<char>(101), static_cast<char>(102)>, 1, 2>"
    )


def test_named_ct_arg_to_json():
    na = gen.NamedArg("def", 1, 2)
    assert na.to_json() == {
        "name": "def",
        "loc": "[1:2]",
    }


def test_named_rt_arg_to_json():
    na = gen.NamedArg("def", 1, 0)
    assert na.to_json() == {
        "name": "def",
        "ref": 1,
    }


def test_named_ct_arg_from_json():
    js = {
        "name": "def",
        "loc": "[1:2]",
    }
    na = gen.NamedArg.from_json(js)
    assert na == gen.NamedArg("def", 1, 2)


def test_named_rt_arg_from_json():
    js = {
        "name": "def",
        "ref": 0,
    }
    na = gen.NamedArg.from_json(js)
    assert na == gen.NamedArg("def", 0, -1)


def test_message_from_cpp_type():
    m = gen.Message.from_cpp_type(
        "sc::message<sc::undefined<sc::args<arg1, arg2>, 42, sc::string<(char)97, (char)98, (char)99>, sc::named_args<sc::named_arg<sc::string<(char)100, (char)101, (char)102>, 1, 2>>>>"
    )
    assert m == gen.Message(
        "abc", ["arg1", "arg2"], 42, [gen.NamedArg("def", 1, 2)], ""
    )


def test_flow_message_type():
    m = gen.Message("flow.step", [], -1, [])
    assert m.type == "flow"


def test_msg_message_type():
    m = gen.Message("step", [], -1, [])
    assert m.type == "msg"


def test_message_to_cpp_type():
    m = gen.Message("abc", ["int"], 42, [gen.NamedArg("def", 1, 2)])
    assert (
        m.to_cpp_type()
        == "sc::message<sc::undefined<sc::args<int>, 42, sc::string<static_cast<char>(97), static_cast<char>(98), static_cast<char>(99)>, sc::named_args<sc::named_arg<sc::string<static_cast<char>(100), static_cast<char>(101), static_cast<char>(102)>, 1, 2>>>>"
    )


def test_message_to_cpp_type_unsigned():
    m = gen.Message("abc", ["int"], 42, [gen.NamedArg("def", 1, 2)], "u")
    assert (
        m.to_cpp_type()
        == "sc::message<sc::undefined<sc::args<int>, 42u, sc::string<static_cast<char>(97), static_cast<char>(98), static_cast<char>(99)>, sc::named_args<sc::named_arg<sc::string<static_cast<char>(100), static_cast<char>(101), static_cast<char>(102)>, 1, 2>>>>"
    )


def test_message_to_json():
    m = gen.Message("abc {}", ["int"], 42, [])
    assert m.to_json() == {
        "msg": "abc {}",
        "type": "msg",
        "arg_types": ["int"],
        "arg_count": 1,
        "id": 42,
        "args": [],
    }


def test_message_from_json():
    js = {
        "msg": "abc {}",
        "type": "msg",
        "arg_types": ["int"],
        "arg_count": 1,
        "id": 42,
        "args": [
            {
                "name": "def",
                "loc": "[1:2]",
            }
        ],
    }
    m = gen.Message.from_json(js)
    assert m == gen.Message("abc {}", ["int"], 42, [gen.NamedArg("def", 1, 2)])


def test_module_from_cpp_type():
    m = gen.Module.from_cpp_type(
        "sc::module_string<sc::undefined<void, 42, sc::string<(char)97, (char)98, (char)99>>>"
    )
    assert m == gen.Module("abc", 42)


def test_module_to_cpp_type():
    m = gen.Module("abc", 42)
    assert (
        m.to_cpp_type()
        == "sc::module_string<sc::undefined<void, 42, sc::string<static_cast<char>(97), static_cast<char>(98), static_cast<char>(99)>>>"
    )


def test_module_to_cpp_type_unsigned():
    m = gen.Module("abc", 42, "u")
    assert (
        m.to_cpp_type()
        == "sc::module_string<sc::undefined<void, 42u, sc::string<static_cast<char>(97), static_cast<char>(98), static_cast<char>(99)>>>"
    )


def test_module_to_json():
    m = gen.Module("abc", 42)
    assert m.to_json() == {"string": "abc", "id": 42}


def test_module_from_json():
    js = {"string": "abc", "id": 42}
    m = gen.Module.from_json(js)
    assert m == gen.Module("abc", 42)


test_ints = "1-5,8-10,15"


def test_intervals():
    m = gen.Intervals(test_ints)
    assert m.contains(1)
    assert m.contains(5)
    assert not m.contains(6)
    assert m.contains(8)
    assert m.contains(10)
    assert m.contains(15)


def test_empty_intervals():
    m = gen.Intervals("")
    assert not m.contains(1)


def test_intervals_repr():
    m = gen.Intervals(test_ints)
    assert f"{m}" == "1-5,8-10,15"
    assert repr(m) == 'Intervals("1-5,8-10,15")'
