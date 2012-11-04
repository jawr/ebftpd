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

util::Error UserProfile::SetExpires(const std::string& value)
{
  if (boost::to_lower_copy(value) == "never")
    expires = boost::optional<boost::gregorian::date>();
  else
  {
    try
    {
      expires = boost::gregorian::from_string(value);
    }
    catch (const std::exception& e)
    {
      return util::Error::Failure("Invalid date. Must be in format YYYY-MM-DD. or NEVER");
    }
  }
  
  return util::Error::Success();
}

// end
}
