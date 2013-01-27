#include <sstream>
#include <cassert>
#include <boost/regex.hpp>
#include "acl/util.hpp"
#include "util/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/usercache.hpp"
#include "util/verify.hpp"
#include "acl/userprofile.hpp"
#include "cfg/get.hpp"

namespace acl
{

namespace
{

boost::regex validationPatterns[] =
{
  boost::regex("\\w+"),             // username
  boost::regex("\\w+"),             // groupname
  boost::regex("[^{}]+")            // tagline
};

}

void CreateDefaults()
{
  if (acl::GroupCache::Create("ebftpd"))
  {
    verify(acl::GroupCache::NameToGID("ebftpd") == 0);
  }
  
  verify(acl::GroupCache::Exists(0));

  if (acl::UserCache::Create("ebftpd", "ebftpd", "1", 0))
  {
    verify(acl::UserCache::NameToUID("ebftpd") == 0);
    verify(acl::UserCache::SetPrimaryGID("ebftpd", 0));
    verify(acl::UserCache::AddIPMask("ebftpd", "*@127.0.0.1"));
  }
  
  verify(acl::UserCache::Exists(0));
  
  if (acl::UserCache::Create("biohazard", "password", "1", 0))
  {
    verify(acl::UserCache::AddIPMask("biohazard", "*@127.0.0.1"));
  }
  
  if (acl::UserCache::Create("io", "password", "1", 0))
  {
    verify(acl::UserCache::AddIPMask("io", "*@127.0.0.1"));
  }
}

bool Validate(ValidationType type, const std::string& s)
{
  return boost::regex_match(s, validationPatterns[static_cast<unsigned>(type)]);
}

std::string FormatRatio(int ratio)
{
  assert(ratio >= 0);
  if (ratio == 0) return "unlimited";
  std::ostringstream os;
  os << "1:" << ratio;
  return os.str();
}

std::string RatioString(const UserProfile& profile)
{
  std::ostringstream os;
  os << FormatRatio(profile.Ratio(""));
  for (const auto& kv : cfg::Get().Sections())
  {
    if (profile.Ratio(kv.first) != -1)
    {
      os << " " << kv.first << "(" << FormatRatio(profile.Ratio(kv.first)) << ")";
    }
  }
  return os.str();
}

std::string FormatCredits(long long credits)
{
  std::ostringstream os;
  os << std::setprecision(2) << std::fixed << (credits / 1024 / 1024.0) << "MB";
  return os.str();
}

std::string CreditString(const UserProfile& profile)
{
  std::ostringstream os;
  os << FormatCredits(profile.Credits(""));
  for (const auto& kv : cfg::Get().Sections())
  {
    if (kv.second.SeparateCredits())
    {
      os << " " << kv.first << "(" << FormatCredits(profile.Credits(kv.first)) << ")";
    }
  }
  return os.str();
}

} /* acl namespace */
