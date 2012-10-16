#include "acl/userprofile.hpp"
#include "acl/usercache.hpp"

namespace acl
{

UserProfile::UserProfile(acl::UserID uid) :
  uid(uid),
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

bool UserProfile::Expired(const std::string& date) const
{
  (void)date;
  return false;
}

util::Error UserProfile::SetRatio(int ratio)
{
  this->ratio = ratio;
  return util::Error::Success();
}

util::Error UserProfile::SetWeeklyAllotment(int weeklyAllotment)
{
  this->weeklyAllotment = weeklyAllotment;
  return util::Error::Success();
}

util::Error UserProfile::SetHomeDir(const std::string& homeDir)
{
  this->homeDir = homeDir;
  return util::Error::Success();
}

util::Error UserProfile::SetStartupDir(const std::string& startupDir)
{
  this->startupDir = startupDir;
  return util::Error::Success();
}

util::Error UserProfile::SetIdleTime(int idleTime)
{
  this->idleTime = idleTime;
  return util::Error::Success();
}

util::Error UserProfile::SetExpires(const std::string& date)
{
  this->expires = boost::gregorian::from_string(date);;
  return util::Error::Success();
}

util::Error UserProfile::SetNumLogins(int numLogins)
{
  this->numLogins = numLogins;
  return util::Error::Success();
}

util::Error UserProfile::SetTagline(const std::string& tagline)
{
  this->tagline = tagline;
  return util::Error::Success();
}

util::Error UserProfile::SetComment(const std::string& comment)
{
  this->comment = comment;
  return util::Error::Success();
}

util::Error UserProfile::SetMaxDlSpeed(int maxDlSpeed)
{
  this->maxDlSpeed = maxDlSpeed;
  return util::Error::Success();
}

util::Error UserProfile::SetMaxUlSpeed(int maxUlSpeed)
{
  this->maxUlSpeed = maxUlSpeed;
  return util::Error::Success();
}

util::Error UserProfile::SetMaxSimDl(int maxSimDl)
{
  this->maxSimDl = maxSimDl;
  return util::Error::Success();
}

util::Error UserProfile::SetMaxSimUl(int maxSimUl)
{
  this->maxSimUl = maxSimUl;
  return util::Error::Success();
}


// end
}
