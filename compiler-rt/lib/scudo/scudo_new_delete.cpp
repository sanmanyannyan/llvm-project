//===-- scudo_new_delete.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// Interceptors for operators new and delete.
///
//===----------------------------------------------------------------------===//

#include "scudo_allocator.h"
#include "scudo_errors.h"

#include "interception/interception.h"

#include <stddef.h>

using namespace __scudo;

// C++ operators can't have dllexport attributes on Windows. We export them
// anyway by passing extra -export flags to the linker, which is exactly that
// dllexport would normally do. We need to export them in order to make the
// VS2015 dynamic CRT (MD) work.
#if SANITIZER_WINDOWS
#define CXX_OPERATOR_ATTRIBUTE
#define COMMENT_EXPORT(sym) __pragma(comment(linker, "/export:" sym))
#ifdef _WIN64
COMMENT_EXPORT("??2@YAPEAX_K@Z")                    // operator new
COMMENT_EXPORT("??2@YAPEAX_KAEBUnothrow_t@std@@@Z") // operator new nothrow
COMMENT_EXPORT("??3@YAXPEAX@Z")                     // operator delete
COMMENT_EXPORT("??3@YAXPEAX_K@Z")                   // sized operator delete
COMMENT_EXPORT("??_U@YAPEAX_K@Z")                   // operator new[]
COMMENT_EXPORT("??_V@YAXPEAX@Z")                    // operator delete[]
#else
COMMENT_EXPORT("??2@YAPAXI@Z")                   // operator new
COMMENT_EXPORT("??2@YAPAXIABUnothrow_t@std@@@Z") // operator new nothrow
COMMENT_EXPORT("??3@YAXPAX@Z")                   // operator delete
COMMENT_EXPORT("??3@YAXPAXI@Z")                  // sized operator delete
COMMENT_EXPORT("??_U@YAPAXI@Z")                  // operator new[]
COMMENT_EXPORT("??_V@YAXPAX@Z")                  // operator delete[]
#endif
#undef COMMENT_EXPORT
#else
#define CXX_OPERATOR_ATTRIBUTE INTERCEPTOR_ATTRIBUTE
#endif

// Fake std::nothrow_t to avoid including <new>.
namespace std {
struct nothrow_t {};
enum class align_val_t: size_t {};
}  // namespace std

// TODO(alekseys): throw std::bad_alloc instead of dying on OOM.
#define OPERATOR_NEW_BODY_ALIGN(Type, Align, NoThrow)              \
  void *Ptr = scudoAllocate(size, static_cast<uptr>(Align), Type); \
  if (!NoThrow && UNLIKELY(!Ptr)) reportOutOfMemory(size);         \
  return Ptr;
#define OPERATOR_NEW_BODY(Type, NoThrow) \
  OPERATOR_NEW_BODY_ALIGN(Type, 0, NoThrow)

CXX_OPERATOR_ATTRIBUTE
void *operator new(size_t size)
{ OPERATOR_NEW_BODY(FromNew, /*NoThrow=*/false); }
CXX_OPERATOR_ATTRIBUTE
void *operator new[](size_t size)
{ OPERATOR_NEW_BODY(FromNewArray, /*NoThrow=*/false); }
CXX_OPERATOR_ATTRIBUTE
void *operator new(size_t size, std::nothrow_t const&)
{ OPERATOR_NEW_BODY(FromNew, /*NoThrow=*/true); }
CXX_OPERATOR_ATTRIBUTE
void *operator new[](size_t size, std::nothrow_t const&)
{ OPERATOR_NEW_BODY(FromNewArray, /*NoThrow=*/true); }
CXX_OPERATOR_ATTRIBUTE
void *operator new(size_t size, std::align_val_t align)
{ OPERATOR_NEW_BODY_ALIGN(FromNew, align, /*NoThrow=*/false); }
CXX_OPERATOR_ATTRIBUTE
void *operator new[](size_t size, std::align_val_t align)
{ OPERATOR_NEW_BODY_ALIGN(FromNewArray, align, /*NoThrow=*/false); }
CXX_OPERATOR_ATTRIBUTE
void *operator new(size_t size, std::align_val_t align, std::nothrow_t const&)
{ OPERATOR_NEW_BODY_ALIGN(FromNew, align, /*NoThrow=*/true); }
CXX_OPERATOR_ATTRIBUTE
void *operator new[](size_t size, std::align_val_t align, std::nothrow_t const&)
{ OPERATOR_NEW_BODY_ALIGN(FromNewArray, align, /*NoThrow=*/true); }

#define OPERATOR_DELETE_BODY(Type) \
  scudoDeallocate(ptr, 0, 0, Type);
#define OPERATOR_DELETE_BODY_SIZE(Type) \
  scudoDeallocate(ptr, size, 0, Type);
#define OPERATOR_DELETE_BODY_ALIGN(Type) \
  scudoDeallocate(ptr, 0, static_cast<uptr>(align), Type);
#define OPERATOR_DELETE_BODY_SIZE_ALIGN(Type) \
  scudoDeallocate(ptr, size, static_cast<uptr>(align), Type);

CXX_OPERATOR_ATTRIBUTE
void operator delete(void *ptr) NOEXCEPT
{ OPERATOR_DELETE_BODY(FromNew); }
CXX_OPERATOR_ATTRIBUTE
void operator delete[](void *ptr) NOEXCEPT
{ OPERATOR_DELETE_BODY(FromNewArray); }
CXX_OPERATOR_ATTRIBUTE
void operator delete(void *ptr, std::nothrow_t const&)
{ OPERATOR_DELETE_BODY(FromNew); }
CXX_OPERATOR_ATTRIBUTE
void operator delete[](void *ptr, std::nothrow_t const&)
{ OPERATOR_DELETE_BODY(FromNewArray); }
CXX_OPERATOR_ATTRIBUTE
void operator delete(void *ptr, size_t size) NOEXCEPT
{ OPERATOR_DELETE_BODY_SIZE(FromNew); }
CXX_OPERATOR_ATTRIBUTE
void operator delete[](void *ptr, size_t size) NOEXCEPT
{ OPERATOR_DELETE_BODY_SIZE(FromNewArray); }
CXX_OPERATOR_ATTRIBUTE
void operator delete(void *ptr, std::align_val_t align) NOEXCEPT
{ OPERATOR_DELETE_BODY_ALIGN(FromNew); }
CXX_OPERATOR_ATTRIBUTE
void operator delete[](void *ptr, std::align_val_t align) NOEXCEPT
{ OPERATOR_DELETE_BODY_ALIGN(FromNewArray); }
CXX_OPERATOR_ATTRIBUTE
void operator delete(void *ptr, std::align_val_t align, std::nothrow_t const&)
{ OPERATOR_DELETE_BODY_ALIGN(FromNew); }
CXX_OPERATOR_ATTRIBUTE
void operator delete[](void *ptr, std::align_val_t align, std::nothrow_t const&)
{ OPERATOR_DELETE_BODY_ALIGN(FromNewArray); }
CXX_OPERATOR_ATTRIBUTE
void operator delete(void *ptr, size_t size, std::align_val_t align) NOEXCEPT
{ OPERATOR_DELETE_BODY_SIZE_ALIGN(FromNew); }
CXX_OPERATOR_ATTRIBUTE
void operator delete[](void *ptr, size_t size, std::align_val_t align) NOEXCEPT
{ OPERATOR_DELETE_BODY_SIZE_ALIGN(FromNewArray); }
