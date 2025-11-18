import gen_str_catalog as gen


def test_split_args():
    assert gen.split_args("1, 2, 3") == ["1", "2", "3"]


def test_split_args_templates():
    assert gen.split_args("array<char, 1>, 2, array<array<char, 3>, 3>") == [
        "array<char, 1>",
        "2",
        "array<array<char, 3>, 3>",
    ]


test_msg = "sc::message<sc::undefined<sc::args<arg1, arg2>, 42, char, (char)97, (char)98, (char)99>>"


def test_message_properties():
    m = gen.Message.from_cpp_type(test_msg)
    assert m.text == "abc"
    assert m.args == ["arg1", "arg2"]
    assert m.id == 42
    assert m.type == "msg"


def test_flow_message_type():
    m = gen.Message("flow.step", [], -1)
    assert m.type == "flow"


def test_message_cpp_type():
    m = gen.Message("abc", ["int"], 42)
    assert (
        m.to_cpp_type()
        == "sc::message<sc::undefined<sc::args<int>, 42, char, static_cast<char>(97), static_cast<char>(98), static_cast<char>(99)>>"
    )


def test_message_json():
    m = gen.Message("abc {}", ["int"], 42)
    assert m.to_json() == {
        "msg": "abc {}",
        "type": "msg",
        "arg_types": ["int"],
        "arg_count": 1,
        "id": 42,
    }


test_module = (
    "sc::module_string<sc::undefined<void, 42, char, (char)97, (char)98, (char)99>>"
)


def test_module_properties():
    m = gen.Module.from_cpp_type(test_module)
    assert m.text == "abc"
    assert m.id == 42


def test_module_cpp_type():
    m = gen.Module("abc", 42)
    assert (
        m.to_cpp_type()
        == "sc::module_string<sc::undefined<void, 42, char, static_cast<char>(97), static_cast<char>(98), static_cast<char>(99)>>"
    )


def test_module_json():
    m = gen.Module("abc", 42)
    assert m.to_json() == {"string": "abc", "id": 42}


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
