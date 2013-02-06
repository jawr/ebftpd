#ifndef __TRANSFERCOUNTER_HPP
#define __TRANSFERCOUNTER_HPP

#include <boost/thread/mutex.hpp>
#include <unordered_map>
#include <functional>
#include "acl/types.hpp"

namespace ftp
{

enum class CounterResult : int;

class TransferCounter
{
  boost::mutex mutex;
  int global;
  std::unordered_map<acl::UserID, int> personal;
  std::function<int(void)> getMaxGlobal;

  TransferCounter(const std::function<int(void)>& getMaxGlobal) :
    global(0), getMaxGlobal(getMaxGlobal)
  { }
  
  TransferCounter& operator=(const TransferCounter&) = delete;
  TransferCounter& operator=(TransferCounter&&) = delete;
  TransferCounter(const TransferCounter&) = delete;
  TransferCounter(TransferCounter&&) = delete;
  
public:
  CounterResult Start(acl::UserID uid, int limit, bool exempt);
  void Stop(acl::UserID uid);
  
  friend class Counter;
};

} /* ftp namespace */

#endif
