mypy_lint(FILES gen_str_catalog.py)

add_unit_test("gen_str_catalog_test" PYTEST FILES "gen_str_catalog_test.py"
              EXTRA_DEPS "gen_str_catalog.py")
