#ifndef __UTIL_ASYNCCRC32_HPP
#define __UTIL_ASYNCCRC32_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <string>
#include <cstdint>
#include <array>
#include <pthread.h>
#include "util/crc32.hpp"

namespace util
{

template <size_t BufferSize, size_t QueueSize>
class AsyncCRC32
{
  typedef std::array<char, BufferSize> DataArray;

  struct Buffer
  {
    size_t len;
    std::atomic<bool> empty;
    DataArray data;
    
    Buffer() : len(0), empty(true) { }
  };

  typedef std::array<Buffer, QueueSize> QueueArray;
  
  std::atomic<bool> finished;
  std::atomic<unsigned> pending;
  boost::thread thread;
  mutable pthread_mutex_t mutex;
  mutable pthread_cond_t readCond;
  mutable pthread_cond_t writeCond;
  CRC32 crc32;
  QueueArray queue;
  typename QueueArray::iterator readIt;
  typename QueueArray::iterator writeIt;
  
  void Main()
  {
    while (true)
    {
      pthread_mutex_lock(&mutex);
      if (readIt->empty)
      {
        if (finished)
        {
          pthread_mutex_unlock(&mutex);
          break;
        }
        pthread_cond_wait(&readCond, &mutex);
      }
      pthread_mutex_unlock(&mutex);

      crc32.Update(readIt->data.data(), readIt->len);

      readIt->empty = true;
      --pending;
      pthread_cond_signal(&writeCond);
      if (++readIt == queue.end()) readIt = queue.begin();
    }
  }
  
   void WaitPending() const
  {
    pthread_mutex_lock(&mutex);
    while (pending > 0) pthread_cond_wait(&writeCond, &mutex);
    pthread_mutex_unlock(&mutex);
  }
  
public:
  AsyncCRC32() : 
    finished(false),
    pending(0),
    readIt(queue.begin()),
    writeIt(queue.begin())
  {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&readCond, nullptr);
    pthread_cond_init(&writeCond, nullptr);
    thread = boost::thread(&AsyncCRC32::Main, this);
  }
  
  ~AsyncCRC32()
  {
    finished = true;
    pthread_cond_signal(&readCond);
    thread.join();
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&readCond);
    pthread_cond_destroy(&writeCond);
  }
  
  void Update(const char* bytes, unsigned len)
  {
    pthread_mutex_lock(&mutex);
    if (!writeIt->empty) pthread_cond_wait(&writeCond, &mutex);
    pthread_mutex_unlock(&mutex);

    std::copy(&bytes[0], &bytes[len], writeIt->data.begin());

    writeIt->len = len;
    writeIt->empty = false;
    ++pending;
    pthread_cond_signal(&readCond);
    if (++writeIt == queue.end()) writeIt = queue.begin();
  }
  
  uint32_t Checksum() const
  {
    WaitPending();
    return crc32.Checksum();
  }
  
  std::string HexString() const
  {
    WaitPending();
    return crc32.HexString();
  }
};

} /* util namespace */

#endif
