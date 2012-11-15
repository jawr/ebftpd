#ifndef __STATS_DAY_HPP
#define __STATS_DAY_HPP

#include "acl/types.hpp"

namespace db { namespace bson {
    class Stat;
  } 
}

namespace stats
{

enum class Direction
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
  long long bytes;
  long long xfertime;

  Direction direction;


public:
  Stat() : files(0), bytes(0), xfertime(0) {}
  ~Stat() {}

  acl::UserID UID() const { return uid; }
  int Day() const { return day; }
  int Week() const { return week; }
  int Month() const { return month; }
  int Year() const { return year; }
  int Files() const { return files; }
  long long Bytes() const { return bytes; }
  long long Xfertime() const { return xfertime; }

  void IncrFiles(int files) { this->files += files; }
  void IncrBytes(long long bytes) { this->bytes += bytes; }
  void IncrXfertime(long long xfertime) { this->xfertime += xfertime; }

  friend class db::bson::Stat;
  
};

// end
}
#endif
