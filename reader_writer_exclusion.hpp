/*
 * Copyright (c) 2017, Ben Smith
 * All rights reserved.
 *
 */

#ifndef __READERWRITEREXCLUSION__
#define __READERWRITEREXCLUSION__

#include <condition_variable>
#include <mutex>
#include <stdexcept>

/**
 * @brief a lock shared between multiple reader threads and a single writer thread
 *
 * Multiple readers are allowed to acquire the lock simultaneously but the writer is blocked until all
 * readers release their locks. Once the writer acquires the lock, readers are blocked from acquiring it
 * until the writer releases.
 */
class ReaderWriterExclusion {
public:
  class ReaderGuard {
  public:
	ReaderGuard(ReaderWriterExclusion &exclusion) : _exclusion(exclusion) {
	  _exclusion.reader_acquire();
	}
	~ReaderGuard() { _exclusion.reader_release(); }

  private:
	ReaderWriterExclusion& _exclusion;
  };

  class WriterGuard {
  public:
	WriterGuard(ReaderWriterExclusion &exclusion) : _exclusion(exclusion) {
	  _exclusion.writer_acquire();
	}
	~WriterGuard() { _exclusion.writer_release(); }

  private:
	ReaderWriterExclusion& _exclusion;
  };

  ReaderWriterExclusion() : _writing(false), _num_readers(0) {}

  void reader_acquire() {
    std::unique_lock<std::mutex> lck(_mtx);
    while (_writing) _cv.wait(lck);
    ++_num_readers;
  }

  void reader_release() {
    std::unique_lock<std::mutex> lck(_mtx);
    --_num_readers;
    _cv.notify_all();
  }

  void writer_acquire() {
    std::unique_lock<std::mutex> lck(_mtx);
    while (_num_readers > 0) _cv.wait(lck);
    if (_writing)
      throw std::runtime_error("Multiple writers are not allowed!");
    _writing = true;
  }

  void writer_release() {
    std::unique_lock<std::mutex> lck(_mtx);
    _writing = false;
    _cv.notify_all();
  }

private:
  std::mutex _mtx;
  std::condition_variable _cv;
  int _num_readers;
  bool _writing;
};

#endif
