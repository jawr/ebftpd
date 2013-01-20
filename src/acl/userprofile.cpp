#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>
#include "acl/userprofile.hpp"
#include "acl/usercache.hpp"

namespace acl
{

bool UserProfile::Expired() const
{
  if (!expires) return false;
  return boost::gregorian::day_clock::local_day() >= *expires;
}

// end
}
