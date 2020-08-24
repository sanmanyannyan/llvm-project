// RUN: %clang_scudo %s -o %t
// RUN: %run %t after  2>&1 | FileCheck %s
// RUN: %run %t before 2>&1 | FileCheck %s

// Test that we hit a guard page when writing past the end of a chunk
// allocated by the Secondary allocator, or writing too far in front of it.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

#ifdef _WIN32
DWORD getsystempagesize() {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}
LONG WINAPI handler(EXCEPTION_POINTERS *ExceptionInfo) {
  fprintf(stderr, "AccessViolation\n");
  ExitProcess(0);
}
#else
void handler(int signo, siginfo_t *info, void *uctx) {
  if (info->si_code == SEGV_ACCERR) {
    fprintf(stderr, "AccessViolation\n");
    exit(0);
  }
  exit(1);
}
long getsystempagesize() {
  return sysconf(_SC_PAGESIZE);
}
#endif

int main(int argc, char **argv)
{
  // The size must be large enough to be serviced by the secondary allocator.
  long page_size = getsystempagesize();
  size_t size = (1U << 19) + page_size;

  assert(argc == 2);

  char *p = (char *)malloc(size);
  assert(p);
  memset(p, 'A', size); // This should not trigger anything.
  // Set up the SIGSEGV handler now, as the rest should trigger an AV.
#ifdef _WIN32
  SetUnhandledExceptionFilter(handler);
#else
  struct sigaction a = {0};
  a.sa_sigaction = handler;
  a.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &a, NULL);
#endif

  if (!strcmp(argv[1], "after")) {
    for (int i = 0; i < page_size; i++)
      p[size + i] = 'A';
  }
  if (!strcmp(argv[1], "before")) {
    for (int i = 1; i < page_size; i++)
      p[-i] = 'A';
  }
  free(p);

  return 1; // A successful test means we shouldn't reach this.
}

// CHECK: AccessViolation
