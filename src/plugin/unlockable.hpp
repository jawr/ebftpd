#ifndef __PLUGIN_UNLOCKABLE_HPP
#define __PLUGIN_UNLOCKABLE_HPP

namespace plugin
{

struct Unlockable
{
  // default is no unlocking -- any members these
  // methods operate on should be mutable
  virtual void Unlock() const { }
	virtual void Lock() const { }
};

class UnlockGuard
{
  const Unlockable& unlockable;
  bool released;
  
public:
  UnlockGuard(const Unlockable& unlockable) : 
    unlockable(unlockable),
    released(false)
  {
    unlockable.Unlock();
  }
  
  ~UnlockGuard()
  {
    if (!released)
    {
      unlockable.Lock();
    }
  }
  
  void Release()
  {
    released = true;
  }
};

} /* script namespace */

#endif
