#include <stdexcept>
#include <boost/algorithm/string/case_conv.hpp>
#include "acl/userprofile.hpp"
#include "acl/usercache.hpp"

namespace acl
{

UserProfile::UserProfile(acl::UserID uid, acl::UserID creator) :
  uid(uid),
  creator(creator),
  ratio(3),
  weeklyAllotment(0),
  homeDir("/"),
  startupDir("/"),
  idleTime(-1),
  numLogins(3),
  tagline("Elite tagline."),
  maxDlSpeed(0),
  maxUlSpeed(0),
  maxSimDl(2),
  maxSimUl(2),
  loggedIn(0)
{
}

bool UserProfile::Expired() const
{
  if (!expires) return false;
  return boost::gregorian::day_clock::local_day() >= *expires;
}

// end
}
