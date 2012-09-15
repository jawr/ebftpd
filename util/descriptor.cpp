#include <cassert>
#include <sys/socket.h>
#include <sys/file.h>
#include "descriptor.hpp"
#include "verify.hpp"

namespace util
{
  descriptor::descriptor() throw() :
    mutex(),
    des(-1),
    locked(false)
  {
  }

  descriptor::descriptor(int des) throw() :
    mutex(),
    des(des),
    locked(false)
  {
  }

  descriptor::~descriptor() throw()
  {
    close();
  }

  descriptor::operator int() throw()
  {
    return des;
  }

  descriptor& descriptor::operator= (int des) throw()
  {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    close();
    this->des = des;
    return *this;
  }

  int descriptor::release() throw()
  {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    int des = this->des;
    this->des = -1;
    return des;
  }

  void descriptor::close() throw()
  {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    if (des >= 0)
    {
      if (locked)
      {
        verify(flock(LOCK_UN));
      }
      ::close(des);
      des = -1;
    }
  }

  bool descriptor::is_open() const throw()
  {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    return des >= 0;
  }

  void descriptor::shutdown(int how) const throw()
  {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    if (des >= 0)
    {
      ::shutdown(des, how);
    }
  }
  
  bool descriptor::flock(int operation)
  {
    if (operation != LOCK_UN)
    {
      if (!::flock(des,operation))
      {
        locked = true;
        return true;
      }
    }
    else
    {
      if (!::flock(des,LOCK_UN))
      {
        locked = false;
        return true;
      }
    }
    return false;
  }
}
