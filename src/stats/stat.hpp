#ifndef __STATS_DAY_HPP
#define __STATS_DAY_HPP

#include "acl/types.hpp"

namespace db { namespace bson {
    class Stat;
  } 
}

namespace stats
{

enum class Direction : uint8_t
{
  Upload,
  Download
};

class Stat
{
  acl::UserID uid;

  int day;
  int week;
  int month;
  int year;

  unsigned int files;
  unsigned int kbytes;
  double xfertime;

  Direction direction;

  ~Stat() {}

public:
  Stat() {}

  acl::UserID UID() const { return uid; }
  int Day() const { return day; }
  int Week() const { return week; }
  int Month() const { return month; }
  int Year() const { return year; }
  unsigned int Files() const { return files; }
  unsigned int Kbytes() const { return kbytes; }
  double Xfertime() const { return xfertime; }

  friend class db::bson::Stat;
  
};

// end
}
#endif
