// the "main" application is a stub that calls into the library code

namespace lib {
auto lib_func() -> void;
}

auto main() -> int { lib::lib_func(); }
