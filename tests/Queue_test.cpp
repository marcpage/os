#include "os/Queue.h"
#include <stdio.h>
#include <thread>

#define dotest(condition)                                                      \
  if (!(condition)) {                                                          \
    fprintf(stderr, "FAIL(%s:%d): %s\n", __FILE__, __LINE__, #condition);      \
  }

#ifdef __Tracer_h__
#define TestIterations 40
#else
#define TestIterations 4000
#endif

void PassTheBuck(exec::Queue<int> &in, exec::Queue<int> &out) {
  try {
    while (true) {
      out.enqueue(in.dequeue());
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  } catch (const exec::Queue<int>::Closed &exception) {
    // expected when no more items
  } catch (const std::exception &exception) {
    fprintf(stderr, "EXCEPTION: %s\n", exception.what());
  }
}

int main(const int /*argc*/, const char *const /*argv*/[]) {
  exec::Queue<int> in(3), out(0, 300);
  std::thread threads[] = {
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out)),
      std::thread(PassTheBuck, std::ref(in), std::ref(out))};
  int fullCount = 0, emptyCount = 0;

  for (int value = 0; value < TestIterations; ++value) {
    if (in.empty()) {
      emptyCount++;
    }
    if (in.full()) {
      fullCount++;
    }
    in.enqueue(value);
    printf("in.size()=%d full=%s\n", in.size(), in.full() ? "true" : "false");
  }
  printf("in queue empty %d times and full %d times\n", emptyCount, fullCount);
  while (in.size() > 0) {
    printf("Queue not empty\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  dotest(in.empty());
  dotest(!in.full());
  dotest(in.size() == 0);
  in.close();
  for (unsigned int thread = 0; thread < sizeof(threads) / sizeof(threads[0]);
       ++thread) {
    threads[thread].join();
  }
  return 0;
}
