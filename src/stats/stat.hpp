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

  int files;
  // might need to store bytes rather than kbytes as we lose files < certain siez
  long long kbytes;
  long long xfertime;

  Direction direction;


public:
  Stat() : files(0), kbytes(0), xfertime(0) {}
  ~Stat() {}

  acl::UserID UID() const { return uid; }
  int Day() const { return day; }
  int Week() const { return week; }
  int Month() const { return month; }
  int Year() const { return year; }
  unsigned int Files() const { return files; }
  long long Kbytes() const { return kbytes; }
  long long Xfertime() const { return xfertime; }

  friend class db::bson::Stat;
  
};

// end
}
#endif
