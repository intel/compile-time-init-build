#pragma once

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define CIB_EVAL0(...) __VA_ARGS__
#define CIB_EVAL1(...) CIB_EVAL0(CIB_EVAL0(CIB_EVAL0(__VA_ARGS__)))
#define CIB_EVAL2(...) CIB_EVAL1(CIB_EVAL1(CIB_EVAL1(__VA_ARGS__)))
#define CIB_EVAL3(...) CIB_EVAL2(CIB_EVAL2(CIB_EVAL2(__VA_ARGS__)))
#define CIB_EVAL4(...) CIB_EVAL3(CIB_EVAL3(CIB_EVAL3(__VA_ARGS__)))
#define CIB_EVAL5(...) CIB_EVAL4(CIB_EVAL4(CIB_EVAL4(__VA_ARGS__)))
#define CIB_EVAL(...) CIB_EVAL5(__VA_ARGS__)

#define CIB_MAP_END(...)
#define CIB_MAP_OUT

#define CIB_EMPTY()
#define CIB_DEFER(id) id CIB_EMPTY()

#define CIB_MAP_GET_END2() 0, CIB_MAP_END
#define CIB_MAP_GET_END1(...) CIB_MAP_GET_END2
#define CIB_MAP_GET_END(...) CIB_MAP_GET_END1
#define CIB_MAP_NEXT0(test, next, ...) next CIB_MAP_OUT
#define CIB_MAP_NEXT1(test, next) CIB_DEFER(CIB_MAP_NEXT0)(test, next, 0)
#define CIB_MAP_NEXT(test, next) CIB_MAP_NEXT1(CIB_MAP_GET_END test, next)
#define CIB_MAP_INC(X) CIB_MAP_INC_##X

#define CIB_MAP0(f, x, peek, ...)                                              \
    f(x) CIB_DEFER(CIB_MAP_NEXT(peek, CIB_MAP1))(f, peek, __VA_ARGS__)
#define CIB_MAP1(f, x, peek, ...)                                              \
    f(x) CIB_DEFER(CIB_MAP_NEXT(peek, CIB_MAP0))(f, peek, __VA_ARGS__)

#define CIB_MAP(f, ...)                                                        \
    CIB_EVAL(CIB_MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))

// NOLINTEND(cppcoreguidelines-macro-usage)
