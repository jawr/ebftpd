#ifndef __STATS_DAY_HPP
#define __STATS_DAY_HPP

#include <cstdint>
#include <type_traits>
#include "stats/types.hpp"
#include "acl/types.hpp"

namespace db { namespace bson
{
struct Stat;
} 
}

namespace stats
{

class Stat
{
  int32_t id;

  int files;
  long long kBytes;
  long long xfertime;
  mutable double speed;

/*  static_assert(std::is_same<decltype(id), acl::UserID>::value, 
                "id must be same as acl::UserID");
  static_assert(std::is_same<decltype(id), acl::GroupID>::value, 
                "id must be same as acl::GroupID");*/
  
public:
  Stat() : id(-1), files(0), kBytes(0), xfertime(0), speed(-1) { }  
  Stat(int32_t id) : id(id), files(0), kBytes(0), xfertime(0), speed(-1) { }

  Stat(int32_t id, int files, long long kBytes, long long xfertime) :
    id(id), files(files), kBytes(kBytes), xfertime(xfertime), speed(-1)
  { } 

  Stat(int32_t id, const Stat& stat) :
    id(id), files(stat.files), kBytes(stat.kBytes), 
    xfertime(stat.xfertime), speed(-1)
  { }

  int32_t ID() const { return id; }
  int Files() const { return files; }
  long long KBytes() const { return kBytes; }
  long long Xfertime() const { return xfertime; }
  
  double Speed() const
  {
    if (speed < 0)
    {
      if (xfertime == 0) speed = kBytes;
      else speed = kBytes / (xfertime / 1000.0);
    }
    return speed;
  }

  void Incr(const Stat& stat)
  {
    files += stat.files;
    kBytes += stat.kBytes;
    xfertime += stat.xfertime;
  }
  
  friend struct db::bson::Stat;
};

// end
}
#endif
