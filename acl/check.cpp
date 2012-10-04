#include <boost/algorithm/string/predicate.hpp>
#include "acl/check.hpp"
#include "acl/user.hpp"
#include "fs/path.hpp"
#include "fs/owner.hpp"

#include <iostream>

extern const std::string dummySiteRoot;

namespace acl
{

namespace PathPermission
{

namespace
{

template <Type type>
struct Traits;

template <>
struct Traits<Upload>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    std::cout << "checking upload" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Resume>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    std::cout << "checking resume" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Makedir>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    std::cout << "checking makedir" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Download>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    std::cout << "checking download" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Dirlog>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    std::cout << "checking dirlog" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Rename>
{
  static util::Error AllowedOwner(const User& user, const std::string& path)
  {
    std::cout << "checking renameown" << std::endl;
    return util::Error::Success();
  }
  
  static util::Error AllowedOther(const User& user, const std::string& path)
  {
    std::cout << "checking rename" << std::endl;
    return util::Error::Success();
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
    std::cout << "checking filemove" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Nuke>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    std::cout << "checking nuke" << std::endl;
    return util::Error::Success();
  }
};

template <>
struct Traits<Delete>
{
  static util::Error AllowedOwner(const User& user, const std::string& path)
  {
    std::cout << "checking deleteown" << std::endl;
    return util::Error::Success();
  }

  static util::Error AllowedOther(const User& user, const std::string& path)
  {
    std::cout << "checking delete" << std::endl;
    return util::Error::Success();
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
    std::cout << "checking view" << std::endl;
    return util::Error::Success();
  }
};

}

bool HiddenFile(const std::string& path)
{
  return boost::ends_with(path, "/" + fs::OwnerFile::ownerFilename);
}

template <Type type>
util::Error Allowed(const User& user, const std::string& path)
{    
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
