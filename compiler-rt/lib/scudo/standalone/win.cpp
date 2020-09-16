//===-- win.cpp --------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "platform.h"

#if SCUDO_WINDOWS

#include "common.h"
#include "win.h"
#include "mutex.h"
#include "string_utils.h"
#include "atomic_helpers.h"

#include <errno.h>
#include <fcntl.h>
//#include <linux/futex.h>
//#include <sched.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/mman.h>
#include <sys/stat.h>
//#include <sys/syscall.h>
//#include <sys/time.h>
#include <time.h>
//#include <unistd.h>

#include <windows.h>
#include <io.h>
#include <psapi.h>
#include <stdlib.h>

// #define needed to link in RtlGenRandom(), a.k.a. SystemFunction036.  See the
// "Community Additions" comment on MSDN here:
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa387694.aspx
#define SystemFunction036 NTAPI SystemFunction036
#include <NTSecAPI.h>
#undef SystemFunction036

namespace scudo {

uptr getPageSize() {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}

void NORETURN die() { abort(); }

void *map(void *Addr, uptr Size, UNUSED const char *Name, uptr Flags,
          UNUSED MapPlatformData *Data) {
    return nullptr;
//  int MmapFlags = MAP_PRIVATE | MAP_ANONYMOUS;
//  int MmapProt;
//  if (Flags & MAP_NOACCESS) {
//    MmapFlags |= MAP_NORESERVE;
//    MmapProt = PROT_NONE;
//  } else {
//    MmapProt = PROT_READ | PROT_WRITE;
//  }
//  if (Addr) {
//    // Currently no scenario for a noaccess mapping with a fixed address.
//    DCHECK_EQ(Flags & MAP_NOACCESS, 0);
//    MmapFlags |= MAP_FIXED;
//  }
//  void *P = mmap(Addr, Size, MmapProt, MmapFlags, -1, 0);
//  if (P == MAP_FAILED) {
//    if (!(Flags & MAP_ALLOWNOMEM) || errno != ENOMEM)
//      dieOnMapUnmapError(errno == ENOMEM);
//    return nullptr;
//  }
//  return P;
}

void unmap(void *Addr, uptr Size, UNUSED uptr Flags,
           UNUSED MapPlatformData *Data) {
//  if (munmap(Addr, Size) != 0)
//    dieOnMapUnmapError();
}

void releasePagesToOS(uptr BaseAddress, uptr Offset, uptr Size,
                      UNUSED MapPlatformData *Data) {
//  void *Addr = reinterpret_cast<void *>(BaseAddress + Offset);
//  while (madvise(Addr, Size, MADV_DONTNEED) == -1 && errno == EAGAIN) {
//}
}

// Calling getenv should be fine (c)(tm) at any time.
const char *getEnv(const char *Name) { return getenv(Name); }

namespace {
enum State : u32 { Unlocked = 0, Locked = 1, Sleeping = 2 };
}

bool HybridMutex::tryLock() {
  return atomic_compare_exchange(&M, Unlocked, Locked) == Unlocked;
}

// The following is based on https://akkadia.org/drepper/futex.pdf.
void HybridMutex::lockSlow() {
//  u32 V = atomic_compare_exchange(&M, Unlocked, Locked);
//  if (V == Unlocked)
//    return;
//  if (V != Sleeping)
//    V = atomic_exchange(&M, Sleeping, memory_order_acquire);
//  while (V != Unlocked) {
//    syscall(SYS_futex, reinterpret_cast<uptr>(&M), FUTEX_WAIT_PRIVATE, Sleeping,
//            nullptr, nullptr, 0);
//    V = atomic_exchange(&M, Sleeping, memory_order_acquire);
//  }
}

void HybridMutex::unlock() {
//  if (atomic_fetch_sub(&M, 1U, memory_order_release) != Locked) {
//    atomic_store(&M, Unlocked, memory_order_release);
//    syscall(SYS_futex, reinterpret_cast<uptr>(&M), FUTEX_WAKE_PRIVATE, 1,
//            nullptr, nullptr, 0);
//  }
}

u64 getMonotonicTime() {
//  timespec TS;
//  clock_gettime(CLOCK_MONOTONIC, &TS);
//  return static_cast<u64>(TS.tv_sec) * (1000ULL * 1000 * 1000) +
//         static_cast<u64>(TS.tv_nsec);
}

u32 getNumberOfCPUs() {
  SYSTEM_INFO sysinfo = {};
  GetNativeSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
}

tid_t getThreadID() {
  return GetCurrentThreadId();
}
#pragma comment(lib, "advapi32.lib")
// Blocking is possibly unused if the getrandom block is not compiled in.
bool getRandom(void *Buffer, uptr Length, UNUSED bool Blocking) {
  if (!Buffer || !Length || Length > 256)
    return false;
  return RtlGenRandom(Buffer, Length) != FALSE;
}

// Allocation free syslog-like API.
extern "C" WEAK int async_safe_write_log(int pri, const char *tag,
                                         const char *msg);

void outputRaw(const char *Buffer) {
//  if (&async_safe_write_log) {
//    constexpr s32 AndroidLogInfo = 4;
//    constexpr uptr MaxLength = 1024U;
//    char LocalBuffer[MaxLength];
//    while (strlen(Buffer) > MaxLength) {
//      uptr P;
//      for (P = MaxLength - 1; P > 0; P--) {
//        if (Buffer[P] == '\n') {
//          memcpy(LocalBuffer, Buffer, P);
//          LocalBuffer[P] = '\0';
//          async_safe_write_log(AndroidLogInfo, "scudo", LocalBuffer);
//          Buffer = &Buffer[P + 1];
//          break;
//        }
//      }
//      // If no newline was found, just log the buffer.
//      if (P == 0)
//        break;
//    }
//    async_safe_write_log(AndroidLogInfo, "scudo", Buffer);
//  } else {
//    write(2, Buffer, strlen(Buffer));
//  }
}

void setAbortMessage(const char *Message) {}

} // namespace scudo

#endif // SCUDO_LINUX
