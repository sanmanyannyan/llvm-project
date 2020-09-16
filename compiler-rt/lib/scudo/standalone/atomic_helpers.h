//===-- atomic_helpers.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef SCUDO_ATOMIC_H_
#define SCUDO_ATOMIC_H_

#include "internal_defs.h"

extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)
extern "C" void _mm_mfence();
#pragma intrinsic(_mm_mfence)
extern "C" void _mm_pause();
#pragma intrinsic(_mm_pause)
extern "C" char _InterlockedExchange8(char volatile *Addend, char Value);
#pragma intrinsic(_InterlockedExchange8)
extern "C" short _InterlockedExchange16(short volatile *Addend, short Value);
#pragma intrinsic(_InterlockedExchange16)
extern "C" long _InterlockedExchange(long volatile *Addend, long Value);
#pragma intrinsic(_InterlockedExchange)
extern "C" long _InterlockedExchangeAdd(long volatile *Addend, long Value);
#pragma intrinsic(_InterlockedExchangeAdd)
extern "C" char _InterlockedCompareExchange8(char volatile *Destination,
                                             char Exchange, char Comparand);
#pragma intrinsic(_InterlockedCompareExchange8)
extern "C" short _InterlockedCompareExchange16(short volatile *Destination,
                                               short Exchange, short Comparand);
#pragma intrinsic(_InterlockedCompareExchange16)
extern "C" long long
_InterlockedCompareExchange64(long long volatile *Destination,
                              long long Exchange, long long Comparand);
#pragma intrinsic(_InterlockedCompareExchange64)
extern "C" void *_InterlockedCompareExchangePointer(void *volatile *Destination,
                                                    void *Exchange,
                                                    void *Comparand);
#pragma intrinsic(_InterlockedCompareExchangePointer)
extern "C" long __cdecl _InterlockedCompareExchange(long volatile *Destination,
                                                    long Exchange,
                                                    long Comparand);
#pragma intrinsic(_InterlockedCompareExchange)

#ifdef _WIN64
extern "C" long long _InterlockedExchangeAdd64(long long volatile *Addend,
                                               long long Value);
#pragma intrinsic(_InterlockedExchangeAdd64)
#endif


namespace scudo {

enum memory_order {
  memory_order_relaxed = 0,
  memory_order_consume = 1,
  memory_order_acquire = 2,
  memory_order_release = 3,
  memory_order_acq_rel = 4,
  memory_order_seq_cst = 5
};
//static_assert(memory_order_relaxed == __ATOMIC_RELAXED, "");
//static_assert(memory_order_consume == __ATOMIC_CONSUME, "");
//static_assert(memory_order_acquire == __ATOMIC_ACQUIRE, "");
//static_assert(memory_order_release == __ATOMIC_RELEASE, "");
//static_assert(memory_order_acq_rel == __ATOMIC_ACQ_REL, "");
//static_assert(memory_order_seq_cst == __ATOMIC_SEQ_CST, "");

struct atomic_u8 {
  typedef u8 Type;
  volatile Type ValDoNotUse;
};

struct atomic_u16 {
  typedef u16 Type;
  volatile Type ValDoNotUse;
};

struct atomic_s32 {
  typedef s32 Type;
  volatile Type ValDoNotUse;
};

struct atomic_u32 {
  typedef u32 Type;
  volatile Type ValDoNotUse;
};

struct atomic_u64 {
  typedef u64 Type;
  // On 32-bit platforms u64 is not necessarily aligned on 8 bytes.
  alignas(8) volatile Type ValDoNotUse;
};

struct atomic_uptr {
  typedef uptr Type;
  volatile Type ValDoNotUse;
};

//template <typename T>
//inline typename T::Type atomic_load(const volatile T *A, memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  typename T::Type V;
//  __atomic_load(&A->ValDoNotUse, &V, MO);
//  return V;
//}
//
//template <typename T>
//inline void atomic_store(volatile T *A, typename T::Type V, memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  __atomic_store(&A->ValDoNotUse, &V, MO);
//}
//
//inline void atomic_thread_fence(memory_order) { __sync_synchronize(); }


INLINE u32 atomic_fetch_add(volatile atomic_u32 *a, u32 v,
                            memory_order mo) {
  (void)mo;
  DCHECK(!((uptr)a % sizeof(*a)));
  return (u32)_InterlockedExchangeAdd((volatile long *)&a->ValDoNotUse,
                                      (long)v);
}

INLINE uptr atomic_fetch_add(volatile atomic_uptr *a, uptr v,
                             memory_order mo) {
  (void)mo;
  DCHECK(!((uptr)a % sizeof(*a)));
#ifdef _WIN64
  return (uptr)_InterlockedExchangeAdd64((volatile long long *)&a->ValDoNotUse,
                                         (long long)v);
#else
  return (uptr)_InterlockedExchangeAdd((volatile long *)&a->ValDoNotUse,
                                       (long)v);
#endif
}



//template <typename T>
//inline typename T::Type atomic_fetch_add(volatile T *A, typename T::Type V,
//                                         memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  return __atomic_fetch_add(&A->ValDoNotUse, V, MO);
//}
//
//template <typename T>
//inline typename T::Type atomic_fetch_sub(volatile T *A, typename T::Type V,
//                                         memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  return __atomic_fetch_sub(&A->ValDoNotUse, V, MO);
//}
//
//template <typename T>
//inline typename T::Type atomic_fetch_and(volatile T *A, typename T::Type V,
//                                         memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  return __atomic_fetch_and(&A->ValDoNotUse, V, MO);
//}
//
//template <typename T>
//inline typename T::Type atomic_fetch_or(volatile T *A, typename T::Type V,
//                                        memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  return __atomic_fetch_or(&A->ValDoNotUse, V, MO);
//}
//
//template <typename T>
//inline typename T::Type atomic_exchange(volatile T *A, typename T::Type V,
//                                        memory_order MO) {
//  DCHECK(!(reinterpret_cast<uptr>(A) % sizeof(*A)));
//  typename T::Type R;
//  __atomic_exchange(&A->ValDoNotUse, &V, &R, MO);
//  return R;
//}
//
template <typename T>
inline bool atomic_compare_exchange_strong(volatile T *A, typename T::Type *Cmp,
                                           typename T::Type Xchg,
                                           memory_order MO) {
  return __atomic_compare_exchange(&A->ValDoNotUse, Cmp, &Xchg, false, MO,
                                   __ATOMIC_RELAXED);
}

INLINE bool atomic_compare_exchange_strong(volatile atomic_u8 *a, u8 *cmp,
                                           u8 xchgv, memory_order mo) {
  (void)mo;
  DCHECK(!((uptr)a % sizeof(*a)));
  u8 cmpv = *cmp;
#ifdef _WIN64
  u8 prev = (u8)_InterlockedCompareExchange8((volatile char *)&a->ValDoNotUse,
                                             (char)xchgv, (char)cmpv);
#else
  u8 prev;
  __asm {
    mov al, cmpv
    mov ecx, a
    mov dl, xchgv
    lock cmpxchg [ecx], dl
    mov prev, al
  }
#endif
  if (prev == cmpv)
    return true;
  *cmp = prev;
  return false;
}

INLINE bool atomic_compare_exchange_strong(volatile atomic_uptr *a,
                                           uptr *cmp, uptr xchg,
                                           memory_order mo) {
  uptr cmpv = *cmp;
  uptr prev = (uptr)_InterlockedCompareExchangePointer(
      (void *volatile *)&a->ValDoNotUse, (void *)xchg, (void *)cmpv);
  if (prev == cmpv)
    return true;
  *cmp = prev;
  return false;
}

INLINE bool atomic_compare_exchange_strong(volatile atomic_u16 *a,
                                           u16 *cmp, u16 xchg,
                                           memory_order mo) {
  u16 cmpv = *cmp;
  u16 prev = (u16)_InterlockedCompareExchange16(
      (volatile short *)&a->ValDoNotUse, (short)xchg, (short)cmpv);
  if (prev == cmpv)
    return true;
  *cmp = prev;
  return false;
}

INLINE bool atomic_compare_exchange_strong(volatile atomic_u32 *a,
                                           u32 *cmp, u32 xchg,
                                           memory_order mo) {
  u32 cmpv = *cmp;
  u32 prev = (u32)_InterlockedCompareExchange((volatile long *)&a->ValDoNotUse,
                                              (long)xchg, (long)cmpv);
  if (prev == cmpv)
    return true;
  *cmp = prev;
  return false;
}

INLINE bool atomic_compare_exchange_strong(volatile atomic_u64 *a,
                                           u64 *cmp, u64 xchg,
                                           memory_order mo) {
  u64 cmpv = *cmp;
  u64 prev = (u64)_InterlockedCompareExchange64(
      (volatile long long *)&a->ValDoNotUse, (long long)xchg, (long long)cmpv);
  if (prev == cmpv)
    return true;
  *cmp = prev;
  return false;
}



template <typename T>
inline bool atomic_compare_exchange_weak(volatile T *A, typename T::Type *Cmp,
                                         typename T::Type Xchg,
                                         memory_order MO) {
  return __atomic_compare_exchange(&A->ValDoNotUse, Cmp, &Xchg, true, MO,
                                   __ATOMIC_RELAXED);
}

// Clutter-reducing helpers.

template <typename T>
inline typename T::Type atomic_load_relaxed(const volatile T *A) {
  return atomic_load(A, memory_order_relaxed);
}

template <typename T>
inline void atomic_store_relaxed(volatile T *A, typename T::Type V) {
  atomic_store(A, V, memory_order_relaxed);
}

template <typename t>
INLINE typename t::type atomic_compare_exchange(volatile t *a,
                                                typename t::type cmp,
                                                typename t::type xchg) {
  atomic_compare_exchange_strong(a, &cmp, xchg, memory_order_acquire);
  return cmp;
}

} // namespace scudo

#endif // SCUDO_ATOMIC_H_
