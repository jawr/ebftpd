#ifndef __FTP_TASK_TASK_HPP
#define __FTP_TASK_TASK_HPP

#include "acl/types.hpp"

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
  KickUser(const acl::UserID& uid) : uid(uid) {}
  void Execute(Listener& listener);
};

// end
}
}

#endif
