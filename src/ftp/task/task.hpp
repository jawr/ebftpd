#ifndef __FTP_TASK_TASK_HPP
#define __FTP_TASK_TASK_HPP

#include <vector>
#include <string>
#include <utility>
#include <boost/thread/future.hpp>
#include "ftp/task/types.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "cfg/config.hpp"

namespace ftp 
{ 

class Server;

namespace task
{

class Task : public std::enable_shared_from_this<Task>
{
public:
  virtual ~Task() { }
  virtual void Execute(Server& server) = 0;
  void Push();
};

class KickUser : public Task
{
  acl::UserID uid;
  bool oneOnly;
  boost::promise<unsigned> promise;
  
public:
  KickUser(acl::UserID uid, boost::unique_future<unsigned>& future, bool oneOnly = false) : 
    uid(uid), oneOnly(oneOnly) { future = promise.get_future(); }
  void Execute(Server& server);
};

class LoginKickUser : public Task
{
public:
  struct Result
  {
    bool kicked;
    boost::posix_time::time_duration idleTime;
    unsigned logins;
    Result() : kicked(false), logins(0) { }
  };
  
private:
  acl::UserID uid;
  boost::promise<Result> promise;
  
public:
  LoginKickUser(acl::UserID uid, boost::unique_future<Result>& future) : uid(uid)
  { future = promise.get_future(); }
  void Execute(Server& server);
};

class GetOnlineUsers : public Task
{
  std::vector<ftp::task::WhoUser>& users;
  boost::promise<bool> promise;
  
public:
  GetOnlineUsers(std::vector<ftp::task::WhoUser>& users, boost::unique_future<bool>& future) : 
    users(users) { future = promise.get_future(); }
    
  void Execute(Server& server);
};

class OnlineUserCount : public Task
{
  int count;
  int allCount;
  boost::promise<void> promise;
  
public:
  OnlineUserCount(boost::unique_future<void>& future) :
    count(0), allCount(0)
  {
    future = promise.get_future();
  }
  
  void Execute(Server& server);
  
  int Count() const { return count; }
  int AllCount() const { return allCount; }
  

};

class ReloadConfig : public Task
{
public:
  enum class Result { Okay, Fail, StopStart };

private:
  boost::unique_future<std::pair<Result, Result>>& future;
  boost::promise<std::pair<Result, Result>> promise;
  
public:
  ReloadConfig(boost::unique_future<std::pair<Result, Result>>& future) : future(future)
  { future = promise.get_future(); }
  
  void Execute(Server& server);
};

class Exit : public Task
{
public:
  void Execute(Server& server);
};

class UserUpdate : public Task
{
  acl::UserID uid;
  
public:
  UserUpdate(acl::UserID uid) : uid(uid) { }
  void Execute(Server& server);
};

// end
}
}

#endif
