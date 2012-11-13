#include <boost/algorithm/string/predicate.hpp>
#include "acl/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"
#include "acl/user.hpp"
#include "logs/logs.hpp"

namespace acl { namespace path
{

namespace
{

bool HiddenFile(const std::string& path)
{
  fs::Path ppath(path);
  std::string dirname(ppath.Dirname().ToString());
  if (dirname[dirname.length() - 1] != '/') dirname += '/';
  std::string basename(ppath.Basename().ToString());

  for (auto& hf : cfg::Get().HiddenFiles())
  {
    if (util::string::WildcardMatch(hf.Path().ToString(), dirname))
    {
      for (auto& mask : hf.Masks())
      {
        if (util::string::WildcardMatch(mask, basename)) return true;
      }
    }
  }
  return false;
}

bool Evaluate(const std::vector<cfg::setting::Right>& rights, 
              const User& user, const std::string& path)
{
  for (const auto& right : rights)
  {
    if (util::string::WildcardMatch(right.Path(), path))
      return right.ACL().Evaluate(user);
  }
  return false;
}

template <Type type>
struct Traits;

template <>
struct Traits<Upload>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Upload(), user, path))
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
    if (Evaluate(cfg::Get().Resume(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Overwrite>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Overwrite(), user, path))
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
    if (Evaluate(cfg::Get().Makedir(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Download>
{
private:
  static util::Error CheckNoretrieve(const std::string& path)
  {
    const cfg::Config& config = cfg::Get();
    std::string basename = fs::Path(path).Basename().ToString();
    for (auto& mask : config.Noretrieve())
    {
      if (util::string::WildcardMatch(mask, basename))
        return util::Error::Failure(EACCES);
    }
    return util::Error::Success();
  }
  
public:
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Download(), user, path))
      return CheckNoretrieve(path);
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Dirlog>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Dirlog(), user, path))
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
    if (Evaluate(cfg::Get().Renameown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
  static util::Error AllowedOther(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Rename(), user, path))
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
    if (Evaluate(cfg::Get().Filemove(), user, path))
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
    if (Evaluate(cfg::Get().Nuke(), user, path))
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
    if (Evaluate(cfg::Get().Deleteown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Delete(), user, path))
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
  static util::Error Allowed(const User&, const std::string& path)
  {
    if (HiddenFile(path)) return util::Error::Failure(ENOENT);
    else return util::Error::Success();
  }
};

template <>
struct Traits<Hideinwho>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Hideinwho(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Freefile>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Freefile(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Nostats>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Nostats(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
};

template <>
struct Traits<Hideowner>
{
  static util::Error Allowed(const User& user, const std::string& path)
  {
    if (Evaluate(cfg::Get().Hideowner(), user, path))
      return util::Error::Success();
   else
      return util::Error::Failure(EACCES);
  }
};

}

bool PrivatePath(const std::string& path, const User& user)
{
  const std::vector<cfg::setting::Privpath>& privPath = 
    cfg::Get().Privpath();
  for (const auto& pp : privPath)
  {
    if (!path.compare(0, pp.Path().Length(), pp.Path()))
      return !pp.ACL().Evaluate(user);
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
  if (boost::ends_with(path, "/" + fs::OwnerFile::ownerFilename)) 
    return util::Error::Failure(ENOENT);
  return Allowed<type>(user, path);
}

template util::Error FileAllowed<Upload>(const User& user, const std::string& path);
template util::Error FileAllowed<Resume>(const User& user, const std::string& path);
template util::Error FileAllowed<Overwrite>(const User& user, const std::string& path);
template util::Error FileAllowed<Download>(const User& user, const std::string& path);
template util::Error FileAllowed<Rename>(const User& user, const std::string& path);
template util::Error FileAllowed<Filemove>(const User& user, const std::string& path);
template util::Error FileAllowed<Delete>(const User& user, const std::string& path);
template util::Error FileAllowed<View>(const User& user, const std::string& path);
template util::Error FileAllowed<Hideinwho>(const User& user, const std::string& path);
template util::Error FileAllowed<Freefile>(const User& user, const std::string& path);
template util::Error FileAllowed<Nostats>(const User& user, const std::string& path);
template util::Error FileAllowed<Hideowner>(const User& user, const std::string& path);

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
template util::Error DirAllowed<Hideinwho>(const User& user, std::string path);
template util::Error DirAllowed<Hideowner>(const User& user, std::string path);

util::Error Filter(const User& user, const std::string& basename, 
    std::string& messagePath)
{
  for (auto& filter : cfg::Get().PathFilter())
  {
    if (filter.ACL().Evaluate(user))
    {
      if (!boost::regex_match(basename, filter.Regex()))
      {
        messagePath = filter.MessagePath();
        return util::Error::Failure(EACCES);
      }
      break;
    }
  }
  return util::Error::Success();
}

} /* path namespace */
} /* acl namespace */