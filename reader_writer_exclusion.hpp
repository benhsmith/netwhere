#ifndef __READERWRITEREXCLUSION__
#define __READERWRITEREXCLUSION__

#include <condition_variable>
#include <mutex>
#include <stdexcept>

class ReaderWriterExclusion {
public:
  ReaderWriterExclusion() : writing(false), num_readers(0) {}

  void reader_acquire() {
    std::unique_lock<std::mutex> lck(mtx);
    while (writing) cv.wait(lck);
    ++num_readers;
  }

  void reader_release() {
    std::unique_lock<std::mutex> lck(mtx);
    --num_readers;
    cv.notify_all();
  }

  void writer_acquire() {
    std::unique_lock<std::mutex> lck(mtx);
    while (num_readers > 0) cv.wait(lck);
    if (writing)
      throw std::runtime_error("Multiple writers are not allowed!");
    writing = true;
  }

  void writer_release() {
    std::unique_lock<std::mutex> lck(mtx);
    writing = false;
    cv.notify_all();
  }

private:
  std::mutex mtx;
  std::condition_variable cv;
  int num_readers;
  bool writing;
};

#endif
