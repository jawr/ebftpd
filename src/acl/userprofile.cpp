#include "acl/userprofile.hpp"
#include "acl/usercache.hpp"

namespace acl
{

UserProfile::UserProfile(acl::UserID uid, acl::UserID creator) :
  uid(uid),
  creator(creator),
  ratio(3),
  homeDir("/"),
  startupDir("/"),
  idleTime(0),
  numLogins(3),
  tagline("Elite tagline."),
  maxDlSpeed(0),
  maxUlSpeed(0),
  maxSimDl(2),
  maxSimUl(2)
{
}

util::Error UserProfile::SetExpires(const std::string& date)
{
  try
  {
    this->expires = boost::gregorian::from_string(date);;
  }
  catch (const boost::bad_lexical_cast& e)
  {
    return util::Error::Failure(e.what());
  }
  return util::Error::Success();
}

// end
}
