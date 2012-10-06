#include <boost/algorithm/string/predicate.hpp>
#include "acl/check.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"

#include <iostream>

namespace acl
{

namespace PathPermission
{

namespace
{

bool Evaluate(const std::vector<cfg::setting::Right>& rights, 
              const User& user, const std::string& path)
{
  for (std::vector<cfg::setting::Right>::const_iterator it =
       rights.begin(); it != rights.end(); ++it)
  {
    if (util::string::WildcardMatch(it->Path(), path))
    {
      return it->ACL().Evaluate(user);
    }
  }
  return true;
}

template <Type type>
struct Traits;

template <>
struct Traits<Upload>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Upload(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Resume>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Resume(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Makedir>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Makedir(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Download>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Download(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Dirlog>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Dirlog(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Rename>
{
  static util::Error AllowedOwner(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Renameown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
  static util::Error AllowedOther(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Rename(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
  static util::Error Allowed(const User& user, const std::string& path)
  {
    fs::Owner owner = fs::OwnerCache::Owner(path);
    if (owner.UID() == user.UID())
      return AllowedOwner(user, path);
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Filemove>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Filemove(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Nuke>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Nuke(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Delete>
{
  static util::Error AllowedOwner(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Deleteown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get()->Delete(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error Allowed(const User& user, const std::string& path)
  {
    fs::Owner owner = fs::OwnerCache::Owner(path);
    if (owner.UID() == user.UID())
      return AllowedOwner(user, path);
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<View>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    // always returns true as all the checking for View is done earlier on
    // as hidden files and priv paths
    return util::Error::Success();
  }
};

}

bool HiddenFile(const std::string& path)
{
  return boost::ends_with(path, "/" + fs::OwnerFile::ownerFilename);
}

bool PrivatePath(const std::string& path, const User& user)
{
  const std::vector<cfg::setting::Privpath>& privPath = 
    cfg::Get()->Privpath();
  for (std::vector<cfg::setting::Privpath>::const_iterator it =
       privPath.begin(); it != privPath.end(); ++it)
  {
    if (!path.compare(0, it->Path().Length(), it->Path()))
      return !it->ACL().Evaluate(user);
  }
  return false;
}

template <Type type>
util::Error Allowed(const User& user, const std::string& path)
{ 
  if (PrivatePath(path, user)) return util::Error::Failure(ENOENT);
  return Traits<type>::Allowed(user, path);
}

template <Type type>
util::Error FileAllowed(const User& user, const std::string& path)
{  
  if (HiddenFile(path)) return util::Error::Failure(ENOENT);
  return Allowed<type>(user, path);
}

template util::Error FileAllowed<Upload>(const User& user, const std::string& path);
template util::Error FileAllowed<Resume>(const User& user, const std::string& path);
template util::Error FileAllowed<Download>(const User& user, const std::string& path);
template util::Error FileAllowed<Rename>(const User& user, const std::string& path);
template util::Error FileAllowed<Filemove>(const User& user, const std::string& path);
template util::Error FileAllowed<Delete>(const User& user, const std::string& path);
template util::Error FileAllowed<View>(const User& user, const std::string& path);

template <Type type>
util::Error DirAllowed(const User& user, std::string path)
{
  if (path.empty()) return util::Error::Failure(EINVAL);
  if (path[path.length() - 1] != '/') path += '/';
  return Allowed<type>(user, path);
}

template util::Error DirAllowed<Makedir>(const User& user,std::string path);
template util::Error DirAllowed<Dirlog>(const User& user, std::string path);
template util::Error DirAllowed<Nuke>(const User& user, std::string path);
template util::Error DirAllowed<Delete>(const User& user, std::string path);
template util::Error DirAllowed<View>(const User& user, std::string path);

} /* PathPermission namespace */

} /* acl namespace */


#ifdef ACL_CHECK_TEST

int main()
{
  using namespace acl;
  User user("test", "test", "1");
  std::string path("/hello/there");

  std::cout << (path + "/") << std::endl;
  
  std::cout << PathPermission::FileAllowed<PathPermission::Upload>(user, path) << std::endl;
  std::cout << PathPermission::DirAllowed<PathPermission::Makedir>(user, path) << std::endl;
  

}

#endif
