#include <sstream>
#include <cassert>
#include <boost/regex.hpp>
#include "acl/util.hpp"
#include "util/error.hpp"
#include "util/verify.hpp"
#include "acl/user.hpp"
#include "cfg/get.hpp"
#include "acl/group.hpp"

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

bool CreateDefaults()
{
  try
  {
    if (!GIDExists(0))
    {
      auto group = Group::Create("ebftpd");
      if (!group || group->ID() != 0) return false;
    }

    if (!UIDExists(0))
    {
      auto user = User::Create("ebftpd", "ebftpd", 0);
      if (!user || user->ID() != 0) return false;      
      user->AddIPMask("*@localhost");
      user->AddFlag(Flag::Siteop);
    }
  }
  catch (const util::RuntimeError&)
  {
    return false;
  }
  
  return true;
}

bool Validate(ValidationType type, const std::string& s)
{
  return boost::regex_match(s, validationPatterns[static_cast<unsigned>(type)]);
}

std::string FormatRatio(int ratio)
{
  assert(ratio >= 0);
  if (ratio == 0) return "Unlimited";
  std::ostringstream os;
  os << "1:" << ratio;
  return os.str();
}

std::string RatioString(const User& user)
{
  std::ostringstream os;
  os << FormatRatio(user.DefaultRatio());
  for (const auto& kv : cfg::Get().Sections())
  {
    if (user.SectionRatio(kv.first) != -1)
    {
      os << " " << kv.first << "(" << FormatRatio(user.SectionRatio(kv.first)) << ")";
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

std::string CreditString(const User& user)
{
  std::ostringstream os;
  os << FormatCredits(user.DefaultCredits());
  for (const auto& kv : cfg::Get().Sections())
  {
    if (kv.second.SeparateCredits())
    {
      os << " " << kv.first << "(" << FormatCredits(user.SectionCredits(kv.first)) << ")";
    }
  }
  return os.str();
}

std::string GroupString(const User& user)
{
  std::ostringstream os;
  if (user.HasGadminGID(user.PrimaryGID())) os << "+";
  os << acl::GIDToName(user.PrimaryGID());
  for (acl::GroupID gid : user.SecondaryGIDs())
  {
    os << " ";
    if (user.HasGadminGID(gid)) os << "+";
    os << acl::GIDToName(gid);
  }
  return os.str();
}

} /* acl namespace */
