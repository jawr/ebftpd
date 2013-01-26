#include <cassert>
#include <boost/regex.hpp>
#include "acl/util.hpp"
#include "util/error.hpp"
#include "acl/groupcache.hpp"
#include "acl/usercache.hpp"
#include "util/verify.hpp"

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
    verify(acl::GroupCache::NameToGID("ebftpd") == 0);
  
  verify(!acl::GroupCache::Exists(0));

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

} /* acl namespace */
