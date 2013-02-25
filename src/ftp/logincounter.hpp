#ifndef __LOGINCOUNTER_HPP
#define __LOGINCOUNTER_HPP

#include <mutex>
#include <unordered_map>
#include <functional>
#include "acl/types.hpp"

namespace ftp
{

enum class CounterResult : int;

class LoginCounter
{
  mutable std::mutex mutex;
  int global;
  std::unordered_map<acl::UserID, int> personal;

  LoginCounter() :
    global(0)
  { }
  
  LoginCounter& operator=(const LoginCounter&) = delete;
  LoginCounter& operator=(LoginCounter&&) = delete;
  LoginCounter(const LoginCounter&) = delete;
  LoginCounter(LoginCounter&&) = delete;
  
public:
  CounterResult Start(acl::UserID uid, int limit, bool kickLogin, bool exempt);
  void Stop(acl::UserID uid);
  int GlobalCount() const;
  
  friend class Counter;
};

} /* ftp namespace */

#endif
