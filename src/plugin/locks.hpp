#ifndef __PLUGIN_LOCKER_HPP
#define __PLUGIN_LOCKER_HPP

#include <boost/thread/tss.hpp>
#include "util/verify.hpp"

namespace plugin
{

struct Locker
{
  virtual ~Locker() { }
	virtual void Lock() = 0;
	virtual void Unlock() = 0;
};

class LockManager
{
  static boost::thread_specific_ptr<Locker> instance;
  
public:
  static void InstallLocker(Locker* locker)
  {
    instance.reset(locker);
  }
  
  static void UninstallLocker()
  {
    instance.reset();
  }
  
  static Locker* Get()
  {
    return instance.get();
  }
};

class LockGuard
{
	Locker* locker;
	bool released;
	
public:
	LockGuard() :
		locker(LockManager::Get()),
		released(false)
	{
    if (locker)
      locker->Lock();
	}
	
	~LockGuard()
	{
		if (locker && !released)
			locker->Unlock();
	}
  
  void Release() { released = true; }
};

class UnlockGuard
{
  Locker* locker;
  bool released;
  
public:
  UnlockGuard() :
    locker(LockManager::Get()),
    released(false)
  {
    if (locker)
      locker->Unlock();
  }
  
  ~UnlockGuard()
  {
    if (locker && !released)
      locker->Lock();
  }
  
  void Release() { released = true; }
};

struct ScopeInstallLocker
{
  ScopeInstallLocker(Locker* locker)
  {
    LockManager::InstallLocker(locker);
  }
  
  ~ScopeInstallLocker()
  {
    LockManager::UninstallLocker();
  }
};

} /* plugin namespace */

#endif
