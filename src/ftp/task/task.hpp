#ifndef __FTP_TASK_TASK_HPP
#define __FTP_TASK_TASK_HPP

#include <vector>
#include <string>
#include <boost/thread/future.hpp>
#include "ftp/task/types.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "cfg/config.hpp"

namespace ftp 
{ 

class Listener;

namespace task
{

class Task
{
public:
  virtual ~Task() {}
  virtual void Execute(Listener& listener) = 0;
};

class KickUser : public Task
{
  acl::UserID uid;
  
public:
  KickUser(acl::UserID uid) : uid(uid) {}
  void Execute(Listener& listener);
};

class GetOnlineUsers : public Task
{
  std::vector<ftp::task::WhoUser>& users;
  boost::unique_future<bool>& future;
  boost::promise<bool> promise;
  
public:
  GetOnlineUsers(std::vector<ftp::task::WhoUser>& users, boost::unique_future<bool>& future) : 
    users(users), future(future) { future = promise.get_future(); }
    
  void Execute(Listener& listener);
};

class ReloadConfig : public Task
{
public:
  enum class Result { Okay, Fail, StopStart };

private:
  boost::unique_future<Result>& future;
  boost::promise<Result> promise;
  
public:
  ReloadConfig(boost::unique_future<Result>& future) : future(future)
  { future = promise.get_future(); }
  
  void Execute(Listener& listener);
};

class Exit : public Task
{
public:
  void Execute(Listener&)
  { }
};

// end
}
}

#endif
