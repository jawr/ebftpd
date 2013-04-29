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

#ifndef __FTP_TASK_TASK_HPP
#define __FTP_TASK_TASK_HPP

#include <vector>
#include <string>
#include <utility>
#include <future>
#include <memory>
#include "ftp/task/types.hpp"
#include "acl/types.hpp"
#include "acl/user.hpp"
#include "cfg/config.hpp"

namespace ftp 
{ 

class Server;
class Client;

namespace task
{

class Task : public std::enable_shared_from_this<Task>
{
  Task& operator=(const Task&) = delete;
  Task(const Task&) = delete;

public:
  Task() = default;
  virtual ~Task() { }
  virtual void Execute(Server& server) = 0;
  void Push();
};

class KickUser : public Task
{
  acl::UserID uid;
  bool oneOnly;
  std::promise<int> promise;
  
public:
  KickUser(acl::UserID uid, std::future<int>& future, bool oneOnly = false) : 
    uid(uid), 
    oneOnly(oneOnly) 
  {
    future = promise.get_future(); 
  }
  
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
  std::promise<Result> promise;
  
public:
  LoginKickUser(acl::UserID uid, std::future<Result>& future) : uid(uid)
  { future = promise.get_future(); }
  void Execute(Server& server);
};

class ReloadConfig : public Task
{
public:
  enum class Result { Okay, Fail, StopStart };

private:
  std::promise<std::pair<Result, Result>> promise;
  
public:
  ReloadConfig(std::future<std::pair<Result, Result>>& future)
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

class ClientFinished : public Task
{
  Client& client;
  
public:
  ClientFinished(Client& client) : client(client) { }
  void Execute(Server& server);
};

// end
}
}

#endif
