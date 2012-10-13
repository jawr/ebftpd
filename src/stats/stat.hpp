#ifndef __STATS_DAY_HPP
#define __STATS_DAY_HPP

#include <ctime>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "acl/user.hpp"

namespace stats
{

enum class Direction : uint8_t
{
  Upload,
  Download
};

class Stat
{
  acl::User user;

  int day;
  int week;
  int month;
  int year;

  unsigned int files;
  unsigned int kBytes;
  double xferTime;

  boost::mutex mtx;

  Direction direction;

protected:

  Stat(const acl::User& user, Direction direction) :
    user(user),
    day(0),
    week(0),
    month(0),
    year(0),
    files(0),
    kBytes(0),
    xferTime(0.0),
    mtx(),
    direction(direction)
  {}

public:

  void Add(unsigned int files, unsigned int kBytes, double time)
  {
    boost::lock_guard<boost::mutex> lock(mtx);
    this->files += files;
    this->kBytes += kBytes;
    this->xferTime += time;
  }

  int Day() const { return day; }
  int Week() const { return week; }
  int Month() const { return month; }
  int Year() const { return year; }
  unsigned int Files() const { return files; }
  unsigned int KBytes() const { return kBytes; }
  double XferTime() const { return xferTime; }
  
};

// end
}
#endif
