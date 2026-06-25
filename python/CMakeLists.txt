mypy_lint(FILES cib/gen_str_catalog.py)

add_unit_test(
    "gen_str_catalog_test" PYTEST FILES "test/gen_str_catalog_test.py"
    EXTRA_DEPS "cib/gen_str_catalog.py")
