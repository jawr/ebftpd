#ifndef __DESCRIPTOR_HPP
#define __DESCRIPTOR_HPP

#include <unistd.h>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

namespace util
{
  class descriptor : boost::noncopyable
  {
    mutable boost::recursive_mutex mutex;
    int des;
    bool locked;

    descriptor(const descriptor& des);
    descriptor& operator= (const descriptor& des);

  public:
    descriptor() throw();
    descriptor(int des) throw();
    ~descriptor() throw();

    operator int() throw();
    descriptor& operator= (int des) throw();
    int release() throw();
    void close() throw();
    bool is_open() const throw();
    void shutdown(int how) const throw();
    bool flock(int operation);
  };

}

#endif
