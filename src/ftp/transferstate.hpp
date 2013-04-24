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

#ifndef __FTP_TRANSFERSTATE_HPP
#define __FTP_TRANSFERSTATE_HPP

#include <ios>
#include <mutex>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace ftp
{

enum class TransferType
{
  Upload,
  Download,
  List,
  None
};

class TransferState
{
  mutable std::mutex mutex;
  TransferType type;
  std::streamsize bytes;
  boost::posix_time::ptime startTime;
  boost::posix_time::ptime endTime;
  
  void Assign(const TransferState& other)
  {
    type = other.type;
    bytes = other.bytes;
    startTime = other.startTime;
    endTime = other.endTime;
  }
  
public:
  TransferState& operator=(const TransferState& rhs)
  {
    std::lock(mutex, rhs.mutex);
    std::lock_guard<std::mutex> lock1(mutex, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(rhs.mutex, std::adopt_lock);
    Assign(rhs);
    return *this;
  }

  TransferState(const TransferState& other)
  {
    std::lock_guard<std::mutex> lock(other.mutex);
    Assign(other);
  }

  TransferState() : type(TransferType::None), bytes(0) { }
  ~TransferState() { Stop(); }
  
  void Start(TransferType type)
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->type = type;
    bytes = 0;
    startTime = boost::posix_time::microsec_clock::local_time();
  }

  void Stop()
  {
    std::lock_guard<std::mutex> lock(mutex);
    type = TransferType::None;
    endTime = boost::posix_time::microsec_clock::local_time();
  }

  void Update(std::streamsize bytes)
  {
    std::lock_guard<std::mutex> lock(mutex);
    this->bytes += bytes;
  }
  
  TransferType Type() const { return type; }
  std::streamsize Bytes() const { return bytes; }
  
  boost::posix_time::ptime StartTime() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return startTime;
  }
  
  boost::posix_time::ptime EndTime() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    return endTime;
  }
  
  boost::posix_time::time_duration Duration() const
  {
    std::lock_guard<std::mutex> lock(mutex);
    if (type == TransferType::None) return endTime - startTime;
    else return boost::posix_time::microsec_clock::local_time() - startTime;
  }
};

} /* ftp namespace */

#endif
