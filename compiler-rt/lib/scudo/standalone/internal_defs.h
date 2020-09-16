//===-- internal_defs.h -----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef SCUDO_INTERNAL_DEFS_H_
#define SCUDO_INTERNAL_DEFS_H_

#include "platform.h"

#include <stdint.h>

#ifndef SCUDO_DEBUG
#define SCUDO_DEBUG 0
#endif

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// String related macros.

#define STRINGIFY_(S) #S
#define STRINGIFY(S) STRINGIFY_(S)
#define CONCATENATE_(S, C) S##C
#define CONCATENATE(S, C) CONCATENATE_(S, C)

// Attributes & builtins related macros.
#if SCUDO_WINDOWS
#if SCUDO_IMPORT_INTERFACE
#define INTERFACE __declspec(dllimport)
#else
#define INTERFACE __declspec(dllexport)
#endif
#else
#define INTERFACE __attribute__((visibility("default")))
#endif

#define HIDDEN __attribute__((visibility("hidden")))

//--------------------------- WEAK FUNCTIONS ---------------------------------//
// When working with weak functions, to simplify the code and make it more
// portable, when possible define a default implementation using this macro:
//
// SCUDO_INTERFACE_WEAK_DEF(<return_type>, <name>, <parameter list>)
//
// For example:
//   SCUDO_INTERFACE_WEAK_DEF(bool, compare, int a, int b) { return a > b; }
//
#if SCUDO_WINDOWS
#include "win_defs.h"
#define SCUDO_INTERFACE_WEAK_DEF(ReturnType, Name, ...)                    \
  WIN_WEAK_EXPORT_DEF(ReturnType, Name, __VA_ARGS__)
#else
#define SCUDO_INTERFACE_WEAK_DEF(ReturnType, Name, ...)                    \
  extern "C" SCUDO_INTERFACE_ATTRIBUTE SANITIZER_WEAK_ATTRIBUTE ReturnType \
      Name(__VA_ARGS__)
#endif

// SCUDO_SUPPORTS_WEAK_HOOKS means that we support real weak functions that
// will evaluate to a null pointer when not defined.
#ifndef SCUDO_SUPPORTS_WEAK_HOOKS
#if SCUDO_LINUX
#define SCUDO_SUPPORTS_WEAK_HOOKS 1
#else
#define SCUDO_SUPPORTS_WEAK_HOOKS 0
#endif
#endif // SCUDO_SUPPORTS_WEAK_HOOKS
// For some weak hooks that will be called very often and we want to avoid the
// overhead of executing the default implementation when it is not necessary,
// we can use the flag SANITIZER_SUPPORTS_WEAK_HOOKS to only define the default
// implementation for platforms that doesn't support weak symbols. For example:
//
//   #if !SANITIZER_SUPPORT_WEAK_HOOKS
//     SANITIZER_INTERFACE_WEAK_DEF(bool, compare_hook, int a, int b) {
//       return a > b;
//     }
//   #endif
//
// And then use it as: if (compare_hook) compare_hook(a, b);
//----------------------------------------------------------------------------//

#if SCUDO_WINDOWS
#define WEAK
#else
#define WEAK __attribute__((weak))
#endif

// Common defs
#ifndef INLINE
#define INLINE inline
#endif

// Platform-specific defs.
#if defined(_MSC_VER)
#define ALWAYS_INLINE __forceinline
#define ALIAS(x)
#define FORMAT(f, a)
#define NOINLINE __declspec(noinline)
#define NORETURN __declspec(noreturn)
#define THREADLOCAL __declspec(thread)
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#define PREFETCH(x) /* _mm_prefetch(x, _MM_HINT_NTA) */ (void)0
#else // _MSC_VER
#define ALWAYS_INLINE inline __attribute__((always_inline))
#define ALIAS(X) __attribute__((alias(X)))
#define FORMAT(F, A) __attribute__((format(printf, F, A)))
#define NOINLINE __attribute__((noinline))
#define NORETURN __attribute__((noreturn))
#define LIKELY(X) __builtin_expect(!!(X), 1)
#define UNLIKELY(X) __builtin_expect(!!(X), 0)
#if defined(__i386__) || defined(__x86_64__)
// __builtin_prefetch(X) generates prefetchnt0 on x86
#define PREFETCH(X) __asm__("prefetchnta (%0)" : : "r"(X))
#else
#define PREFETCH(X) __builtin_prefetch(X)
#endif
#endif // _MSC_VER

#if !defined(_MSC_VER) || defined(__clang__)
#define UNUSED __attribute__((unused))
#define USED __attribute__((used))
#else
#define UNUSED
#define USED
#endif

#if !defined(_MSC_VER) || defined(__clang__) || MSC_PREREQ(1900)
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif
namespace scudo {

#if defined(_WIN64)
// 64-bit Windows uses LLP64 data model.
typedef unsigned long long uptr;
typedef signed long long sptr;
#else
typedef unsigned long uptr;
typedef signed long sptr;
#endif  // defined(_WIN64)
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


typedef u64 tid_t;

// The following two functions have platform specific implementations.
void outputRaw(const char *Buffer);
void NORETURN die();

#define RAW_CHECK_MSG(Expr, Msg)                                               \
  do {                                                                         \
    if (UNLIKELY(!(Expr))) {                                                   \
      outputRaw(Msg);                                                          \
      die();                                                                   \
    }                                                                          \
  } while (false)

#define RAW_CHECK(Expr) RAW_CHECK_MSG(Expr, #Expr)

void NORETURN reportCheckFailed(const char *File, int Line,
                                const char *Condition, u64 Value1, u64 Value2);

#define CHECK_IMPL(C1, Op, C2)                                                 \
  do {                                                                         \
    scudo::u64 V1 = (scudo::u64)(C1);                                          \
    scudo::u64 V2 = (scudo::u64)(C2);                                          \
    if (UNLIKELY(!(V1 Op V2))) {                                               \
      scudo::reportCheckFailed(__FILE__, __LINE__,                             \
                               "(" #C1 ") " #Op " (" #C2 ")", V1, V2);         \
      scudo::die();                                                            \
    }                                                                          \
  } while (false)

#define CHECK(A) CHECK_IMPL((A), !=, 0)
#define CHECK_EQ(A, B) CHECK_IMPL((A), ==, (B))
#define CHECK_NE(A, B) CHECK_IMPL((A), !=, (B))
#define CHECK_LT(A, B) CHECK_IMPL((A), <, (B))
#define CHECK_LE(A, B) CHECK_IMPL((A), <=, (B))
#define CHECK_GT(A, B) CHECK_IMPL((A), >, (B))
#define CHECK_GE(A, B) CHECK_IMPL((A), >=, (B))

#if SCUDO_DEBUG
#define DCHECK(A) CHECK(A)
#define DCHECK_EQ(A, B) CHECK_EQ(A, B)
#define DCHECK_NE(A, B) CHECK_NE(A, B)
#define DCHECK_LT(A, B) CHECK_LT(A, B)
#define DCHECK_LE(A, B) CHECK_LE(A, B)
#define DCHECK_GT(A, B) CHECK_GT(A, B)
#define DCHECK_GE(A, B) CHECK_GE(A, B)
#else
#define DCHECK(A)
#define DCHECK_EQ(A, B)
#define DCHECK_NE(A, B)
#define DCHECK_LT(A, B)
#define DCHECK_LE(A, B)
#define DCHECK_GT(A, B)
#define DCHECK_GE(A, B)
#endif

// The superfluous die() call effectively makes this macro NORETURN.
#define UNREACHABLE(Msg)                                                       \
  do {                                                                         \
    CHECK(0 && Msg);                                                           \
    die();                                                                     \
  } while (0)

} // namespace scudo

#endif // SCUDO_INTERNAL_DEFS_H_
