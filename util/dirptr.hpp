#ifndef __DIRPTR_HPP
#define __DIRPTR_HPP

#include <dirent.h>
#include <boost/noncopyable.hpp>

namespace util
{
  class dirptr : boost::noncopyable
  {
    DIR* dp;

    dirptr(const dirptr& dp);
    dirptr& operator= (const dirptr& dp);

  public:
    dirptr() throw() :
      dp(0)
    {
    }

    dirptr(DIR* dp) throw() :
      dp(dp)
    {
    }

    ~dirptr() throw()
    {
      closedir(dp);
    }

    operator DIR*() throw()
    {
      return dp;
    }

    dirptr& operator= (DIR* dp) throw()
    {
      close();
      this->dp = dp;
      return *this;
    }

    DIR* release() throw()
    {
      DIR* dp = this->dp;
      this->dp = 0;
      return dp;
    }

    void close() throw()
    {
      if (dp)
      {
        closedir(dp);
        dp = 0;
      }
    }
  };
}

#endif
