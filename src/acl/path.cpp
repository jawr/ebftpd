//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/algorithm/string/replace.hpp>
#include <boost/regex.hpp>
#include "acl/path.hpp"
#include "fs/owner.hpp"
#include "cfg/get.hpp"
#include "util/string.hpp"
#include "acl/user.hpp"
#include "logs/logs.hpp"
#include "util/path/status.hpp"

namespace acl { namespace path
{

namespace
{

bool HiddenFile(const fs::VirtualPath& path)
{
  std::string dirname(path.Dirname().ToString());
  if (dirname[dirname.length() - 1] != '/') dirname += '/';
  std::string basename(path.Basename().ToString());

  for (auto& hf : cfg::Get().HiddenFiles())
  {
    if (util::WildcardMatch(hf.Path(), dirname))
    {
      for (auto& mask : hf.Masks())
      {
        if (util::WildcardMatch(mask, basename)) return true;
      }
    }
  }
  return false;
}

bool Evaluate(const std::vector<cfg::Right>& rights, 
              const User& user, const fs::VirtualPath& path)
{
  std::string group;
  bool firstSpecial = true;
  auto info = user.ACLInfo();

  for (const auto& right : rights)
  {
    if (right.SpecialVar())
    {
      std::string specialPath(right.Path());
      boost::replace_all(specialPath, "[:username:]", user.Name());
      if (firstSpecial)
      {
        if (user.PrimaryGID() != -1)
        {
          group = user.PrimaryGroup();
        }
        firstSpecial = false;
      }
      
      if (!group.empty())
        boost::replace_all(specialPath, "[:groupname:]", group);
        
      if (util::WildcardMatch(specialPath, path.ToString()))
        return right.ACL().Evaluate(info);
    }
    else
      if (util::WildcardMatch(right.Path(), path.ToString()))
        return right.ACL().Evaluate(info);
  }
  return false;
}

template <Type type>
struct Traits;

template <>
struct Traits<Upload>
{
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
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
private:
  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Resumeown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Resume(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Overwrite>
{
private:
  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Overwriteown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Overwrite(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Makedir>
{
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
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
  static util::Error CheckNoretrieve(const fs::VirtualPath& path)
  {
    const cfg::Config& config = cfg::Get();
    for (auto& mask : config.Noretrieve())
    {
      if (util::WildcardMatch(mask, path.Basename().ToString()))
        return util::Error::Failure(EACCES);
    }
    return util::Error::Success();
  }

  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Downloadown(), user, path))
      return CheckNoretrieve(path);
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Download(), user, path))
      return CheckNoretrieve(path);
    else
      return util::Error::Failure(EACCES);
  }  
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Rename>
{
private:
  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Renameown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Rename(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Filemove>
{
private:
  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Filemoveown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Filemove(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Modify>
{
private:
  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Modifyown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Modify(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<Nuke>
{
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
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
private:
  static util::Error AllowedOwner(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Deleteown(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }

  static util::Error AllowedOther(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Delete(), user, path))
      return util::Error::Success();
    else
      return util::Error::Failure(EACCES);
  }
  
public:
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    fs::Owner owner = GetOwner(fs::MakeReal(path));
    if (owner.UID() == user.ID() && AllowedOwner(user, path))
      return util::Error::Success();
    else
      return AllowedOther(user, path);
  }
};

template <>
struct Traits<View>
{
  static util::Error Allowed(const User&, const fs::VirtualPath& path)
  {
    if (HiddenFile(path)) return util::Error::Failure(ENOENT);
    else return util::Error::Success();
  }
};

template <>
struct Traits<Hideinwho>
{
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
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
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
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
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
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
  static util::Error Allowed(const User& user, const fs::VirtualPath& path)
  {
    if (Evaluate(cfg::Get().Hideowner(), user, path))
      return util::Error::Success();
   else
      return util::Error::Failure(EACCES);
  }
};

}

bool PrivatePath(const fs::VirtualPath& path, const User& user)
{
  auto info = user.ACLInfo();
  for (const auto& pp : cfg::Get().Privpath())
  {
    if (!path.ToString().compare(0, pp.Path().length(), pp.Path()))
      return !pp.ACL().Evaluate(info);
  }
  return false;
}

bool InsideHomeDir(const User& user, const fs::VirtualPath& path)
{
  if (user.HomeDir().empty()) return false;
  if (user.HomeDir().back() == '/') return util::StartsWith(path.ToString(), user.HomeDir());
  else return util::StartsWith(path.ToString(), user.HomeDir() + '/');
}

template <Type type>
util::Error InnerAllowed(const User& user, const fs::VirtualPath& path)
{ 
  if (!InsideHomeDir(user, path)) return util::Error::Failure(EACCES);
  if (PrivatePath(path, user)) return util::Error::Failure(ENOENT);
  return Traits<type>::Allowed(user, path);
}

template <Type type>
util::Error FileAllowed(const User& user, const fs::VirtualPath& path)
{  
  return InnerAllowed<type>(user, path);
}

template util::Error FileAllowed<Upload>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Resume>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Overwrite>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Download>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Rename>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Filemove>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Delete>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<View>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Hideinwho>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Freefile>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Nostats>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Hideowner>(const User& user, const fs::VirtualPath& path);
template util::Error FileAllowed<Modify>(const User& user, const fs::VirtualPath& path);

template <Type type>
util::Error DirAllowed(const User& user, const fs::VirtualPath& path)
{
  if (path.IsEmpty()) return util::Error::Failure(EINVAL);
  if (path.ToString()[path.Length() - 1] != '/') 
    return InnerAllowed<type>(user, fs::VirtualPath(path.ToString() + '/'));
  else
    return InnerAllowed<type>(user, path);
}

template util::Error DirAllowed<Makedir>(const User& user, const fs::VirtualPath& path);
template util::Error DirAllowed<Rename>(const User& user, const fs::VirtualPath& path);
template util::Error DirAllowed<Nuke>(const User& user, const fs::VirtualPath& path);
template util::Error DirAllowed<Delete>(const User& user, const fs::VirtualPath& path);
template util::Error DirAllowed<View>(const User& user, const fs::VirtualPath& path);
template util::Error DirAllowed<Hideinwho>(const User& user, const fs::VirtualPath& path);
template util::Error DirAllowed<Hideowner>(const User& user, const fs::VirtualPath& path);

template <Type type>
util::Error Allowed(const User& user, const fs::VirtualPath& path)
{
  if (util::path::IsDirectory(fs::MakeReal(path).ToString()))
    return DirAllowed<type>(user, path);
  else
    return FileAllowed<type>(user, path);
}

template util::Error Allowed<View>(const User& user, const fs::VirtualPath& path);

util::Error Filter(const User& user, const fs::Path& basename)
{
  auto info = user.ACLInfo();
  for (auto& filter : cfg::Get().PathFilter())
  {
    if (filter.ACL().Evaluate(info))
    {
      if (!boost::regex_match(basename.ToString(), filter.Regex()))
      {
        return util::Error::Failure(EACCES);
      }
      break;
    }
  }
  return util::Error::Success();
}

} /* path namespace */
} /* acl namespace */
