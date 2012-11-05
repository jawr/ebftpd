#ifndef __FTP_TASK_TASK_HPP
#define __FTP_TASK_TASK_HPP

#include <vector>
#include <string>
#include <boost/thread/future.hpp>
#include "ftp/task/types.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"

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
  boost::unique_future<bool>& future;
  boost::promise<bool> promise;
  
public:
  ReloadConfig(boost::unique_future<bool>& future) : future(future)
  { future = promise.get_future(); }
  
  void Execute(Listener& listener);
};

// end
}
}

#endif
