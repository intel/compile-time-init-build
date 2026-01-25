import json
import log_decode as decode
from pathlib import Path


catalog_output = Path("./catalog_test.bin")
json_filename = Path("./strings.json")


expected_lines = {
    "Zero arguments",
    "One compile-time argument: 17",
    "Fixed string_id (1337)",
    "Fixed unsigned string_id (1338)",
    "TRACE [default] One int32_t runtime argument: 17",
    "TRACE [default] One int64_t runtime argument: 17",
    "TRACE [default] One int32_t runtime argument formatted as {:08x}: 00000011",
    "TRACE [default] Two runtime arguments: uint32_t 1 and int64_t 2",
    "TRACE [default] Float argument: 3.140000104904175",
    "TRACE [default] Double argument: 3.14",
    "TRACE [default] Default module with runtime argument: 17",
    'TRACE [not default] Overridden module ("not default") with runtime argument: 17',
    'TRACE [fixed] Fixed module ("fixed") with runtime argument: 17',
    'TRACE [fixed_id6] Fixed module_id (6) and module ("fixed_id6") with runtime argument: 17',
    'TRACE [fixed_id7] Fixed unsigned module_id (7) and module ("fixed_id7") with runtime argument: 17',
    "TRACE [default] Auto-declared scoped enum argument: static_cast<some_ns::E>(17)",
}


def test_files_exist():
    assert catalog_output.is_file()
    assert json_filename.is_file()


def test_binary_logs():
    try:
        import clang.cindex

        expected_lines.add("TRACE [default] Scoped enum argument: VAL_E1")
        expected_lines.add("TRACE [default] Unscoped enum argument: VAL_E2")
    except ModuleNotFoundError:
        expected_lines.add(
            "TRACE [default] Scoped enum argument: static_cast<ns::E1>(19)"
        )
        expected_lines.add(
            "TRACE [default] Unscoped enum argument: static_cast<ns::E2>(23)"
        )

    with open(json_filename, "r") as f:
        db = json.load(f)
    for log_line in decode.read_logs(catalog_output, db):
        assert log_line in expected_lines
        expected_lines.remove(log_line)
    assert not expected_lines
