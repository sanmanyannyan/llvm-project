// RUN: %clangxx_scudo %s -o %t
// RUN: %env_scudo_opts="QuarantineSizeKb=0:ThreadLocalQuarantineSizeKb=0"     %run %t 5 1000000 2>&1
// RUN: %env_scudo_opts="QuarantineSizeKb=1024:ThreadLocalQuarantineSizeKb=64" %run %t 5 1000000 2>&1

// Tests parallel allocations and deallocations of memory chunks from a number
// of concurrent threads, with and without quarantine.
// This test passes if everything executes properly without crashing.

#include <assert.h>
#include <condition_variable>
#include <stdio.h>
#include <stdlib.h>
#include <thread>

#include <sanitizer/allocator_interface.h>

int num_threads;
int total_num_alloc;
const int kMaxNumThreads = 500;
std::thread thread[kMaxNumThreads];

std::condition_variable cond;
std::mutex mutex;
char go = 0;

void *thread_fun(void *arg) {
  mutex.lock();
  while (!go) {
    std::unique_lock<std::mutex> lk(mutex);
    cond.wait(lk);
  }

  mutex.unlock();
  for (int i = 0; i < total_num_alloc / num_threads; i++) {
    void *p = malloc(10);
    __asm__ __volatile__(""
                         :
                         : "r"(p)
                         : "memory");
    free(p);
  }
  return 0;
}

int main(int argc, char **argv) {
  assert(argc == 3);
  num_threads = atoi(argv[1]);
  assert(num_threads > 0);
  assert(num_threads <= kMaxNumThreads);
  total_num_alloc = atoi(argv[2]);
  assert(total_num_alloc > 0);

  printf("%d threads, %d allocations in each\n", num_threads,
         total_num_alloc / num_threads);
  fprintf(stderr, "Heap size before: %zd\n", __sanitizer_get_heap_size());
  fprintf(stderr, "Allocated bytes before: %zd\n",
          __sanitizer_get_current_allocated_bytes());

  mutex.lock();
  for (int i = 0; i < num_threads; i++)
    thread[i] = std::thread(thread_fun, (void *)0);
  go = 1;
  cond.notify_all();
  mutex.unlock();
  for (int i = 0; i < num_threads; i++)
    thread[i].join();

  fprintf(stderr, "Heap size after: %zd\n", __sanitizer_get_heap_size());
  fprintf(stderr, "Allocated bytes after: %zd\n",
          __sanitizer_get_current_allocated_bytes());

  return 0;
}
