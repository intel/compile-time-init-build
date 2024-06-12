#include <cib/func_decl.hpp>

#include <string>

extern std::string actual;

template <> auto cib_func<"e">() -> void { actual += "e"; }
template <> auto cib_func<"f">() -> void { actual += "f"; }
