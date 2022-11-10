#pragma once

#ifndef __cpp_constinit
#if defined(__clang__)
#define CIB_CONSTINIT [[clang::require_constant_initialization]]
#else
#define CIB_CONSTINIT
#endif
#else
#define CIB_CONSTINIT constinit
#endif

#ifndef __cpp_consteval
#define CIB_CONSTEVAL constexpr
#else
#define CIB_CONSTEVAL consteval
#endif

#define CIB_CONSTEXPR constexpr
