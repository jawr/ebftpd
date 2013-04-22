//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __UTIL_ASYNCCRC32_HPP
#define __UTIL_ASYNCCRC32_HPP

#include <memory>
#include <algorithm>
#include <cassert>
#include <string>
#include <cstdint>
#include <array>
#include <boost/thread/thread.hpp>
#include <mutex>
#include <condition_variable>
#include "util/crc32.hpp"

namespace util
{

class AsyncCRC32 : public CRC32
{
  typedef std::vector<uint8_t> DataVec;

  struct Buffer
  {
    size_t len;
    bool empty;
    DataVec data;
    
    Buffer(size_t bufferSize) : 
      len(0), empty(true)
    {
      data.resize(bufferSize);
    }
  };

  typedef std::vector<Buffer*> QueueVec;
  
  bool finished;
  unsigned pending;
  mutable std::mutex mutex;
  mutable std::condition_variable readCond;
  mutable std::condition_variable writeCond;
  QueueVec queue;
  typename QueueVec::iterator readIt;
  typename QueueVec::iterator writeIt;
  boost::thread thread;
  
  void Main()
  {
    while (true)
    {
      {
        std::unique_lock<std::mutex> lock(mutex);
        if ((*readIt)->empty)
        {
          if (finished) goto exitloop;
          while (true)
          {
            readCond.wait(lock);
            if (!(*readIt)->empty) break;
            if (finished) goto exitloop;
          }
        }
      }
      
      CRC32::Update((*readIt)->data.data(), (*readIt)->len);

      mutex.lock();
      (*readIt)->empty = true;
      --pending;
      assert(pending <= queue.size());
      mutex.unlock();

      writeCond.notify_one();
      if (++readIt == queue.end()) readIt = queue.begin();
    }
    
  exitloop:
    return;
  }
  
  void WaitPending() const
  {
    std::unique_lock<std::mutex> lock(mutex);
    while (pending > 0) writeCond.wait(lock);
  }
  
public:
  AsyncCRC32(size_t bufferSize, unsigned queueSize) : 
    finished(false),
    pending(0)
  {
    while (queue.size() < queueSize)
    { 
      queue.emplace_back(new Buffer(bufferSize));
    }
    
    readIt = queue.begin();
    writeIt = queue.begin();
    thread = boost::thread(&AsyncCRC32::Main, this);
  }
  
  ~AsyncCRC32()
  {
    mutex.lock();
    finished = true;
    mutex.unlock();
    
    readCond.notify_one();
    thread.join();
    for (auto buf : queue) delete buf;
  }

  void Update(unsigned len)
  {
    assert(len <= (*writeIt)->data.size());

    mutex.lock();
    (*writeIt)->len = len;
    (*writeIt)->empty = false;    
    ++pending;
    assert(pending <= queue.size());
    mutex.unlock();
    
    readCond.notify_one();
    
    if (++writeIt == queue.end()) writeIt = queue.begin();
    std::unique_lock<std::mutex> lock(mutex);
    while (!(*writeIt)->empty) writeCond.wait(lock);
  }

  uint8_t* GetBuffer()
  {
    return (*writeIt)->data.data();
  }
  
  void Update(const uint8_t* bytes, unsigned len)
  {
    assert(len <= (*writeIt)->data.size());
    
    {
      std::unique_lock<std::mutex> lock(mutex);
      while (!(*writeIt)->empty) writeCond.wait(lock);
    }

    std::copy(&bytes[0], &bytes[len], (*writeIt)->data.begin());

    (*writeIt)->len = len;

    mutex.lock();
    (*writeIt)->empty = false;    
    ++pending;
    mutex.unlock();
    
    readCond.notify_one();
    if (++writeIt == queue.end()) writeIt = queue.begin();
  }
  
  uint32_t Checksum() const
  {
    WaitPending();
    return CRC32::Checksum();
  }
  
  std::string HexString() const
  {
    WaitPending();
    return CRC32::HexString();
  }
};

} /* util namespace */

#endif
