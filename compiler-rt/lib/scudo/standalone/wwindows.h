//===-- wwindows.h ----------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef SCUDO_WWINDOWS_H_
#define SCUDO_WWINDOWS_H_

#include "platform.h"

#if SCUDO_WINDOWS

namespace scudo {

// MapPlatformData is unused on Windows, define it as a minimally sized structure.
struct MapPlatformData {};

} // namespace scudo

#endif // SCUDO_WINDOWS

#endif // SCUDO_WWINDOWS_H_
