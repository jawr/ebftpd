#ifndef __FTP_TRANSFERSTATE_HPP
#define __FTP_TRANSFERSTATE_HPP

#include <ios>
#include <boost/thread/mutex.hpp>
//#include <boost/thread/lock_algorithms.hpp>
#include <boost/thread/locks.hpp>
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
  mutable boost::mutex mutex;
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
    boost::lock(mutex, rhs.mutex);
    boost::lock_guard<boost::mutex> lock1(mutex, boost::adopt_lock);
    boost::lock_guard<boost::mutex> lock2(rhs.mutex, boost::adopt_lock);
    Assign(rhs);
    return *this;
  }

  TransferState(const TransferState& other)
  {
    boost::lock_guard<boost::mutex> lock(other.mutex);
    Assign(other);
  }

  TransferState() : type(TransferType::None), bytes(0) { }
  ~TransferState() { Stop(); }
  
  void Start(TransferType type)
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    this->type = type;
    bytes = 0;
    startTime = boost::posix_time::microsec_clock::local_time();
  }

  void Stop()
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    type = TransferType::None;
    endTime = boost::posix_time::microsec_clock::local_time();
  }

  void Update(std::streamsize bytes)
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    this->bytes += bytes;
  }
  
  TransferType Type() const { return type; }
  std::streamsize Bytes() const { return bytes; }
  
  boost::posix_time::ptime StartTime() const
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    return startTime;
  }
  
  boost::posix_time::ptime EndTime() const
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    return endTime;
  }
  
  boost::posix_time::time_duration Duration() const
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    if (type == TransferType::None) return endTime - startTime;
    else return boost::posix_time::microsec_clock::local_time() - startTime;
  }
};

} /* ftp namespace */

#endif
