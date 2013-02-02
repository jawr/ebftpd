#include <iomanip>
#include <ctime>
#include <cmath>
#include <memory>
#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <fnmatch.h>
#include <boost/tokenizer.hpp>
#include "cmd/dirlist.hpp"
#include "ftp/client.hpp"
#include "fs/direnumerator.hpp"
#include "fs/status.hpp"
#include "acl/usercache.hpp"
#include "acl/groupcache.hpp"
#include "logs/logs.hpp"

namespace cmd
{
namespace
{

enum Options
{
  OptAll          = 'a',  // do not ignore entries starting with .
  OptAlmostAll    = 'A',  // do not list implied . and ..
  OptLongFormat   = 'l',  // use a long listing format
  OptSlashDirs    = 'p',  // append slash to end of directories
  OptReverse      = 'r',  // reverse order while sorting
  OptRecursive    = 'R',  // list subdirectories recursively
  OptSizeSort     = 'S',  // sort by file size
  OptModTimeSort  = 't',  // sort by modification time, newest first
  OptNoGroup      = 'o',  // skip group in long format
  OptNoOwners     = 'y',  // don't load owners
  OptSizeName     = 'z'   // display size and name only
};

}

ListOptions::ListOptions(const std::string& userDefined,
                         const std::string& forced) :
  all(false),
  longFormat(false),
  slashDirs(false),
  reverse(false),
  recursive(false),
  sizeSort(false),
  modTimeSort(false),
  noGroup(false),
  sizeName(false),
  noOwners(false)
{
  std::string combined(forced);
  combined += userDefined;

  for (char ch : combined) ParseOption(ch);
}

void ListOptions::ParseOption(char option)
{
  switch (option)
  {
    case OptAll         :
    {
      all = true;
      break;
    }
    case OptAlmostAll   :
    {
      all = true;
      break;
    }
    case OptLongFormat  :
    {
      longFormat = true;
      break;
    }
    case OptSlashDirs   :
    {
      slashDirs = true;
      break;
    }
    case OptReverse     :
    {
      reverse = true;
      break;
    }
    case OptRecursive   :
    {
      recursive = true;
      break;
    }
    case OptSizeSort    :
    {
      sizeSort = true;
      modTimeSort = false;
      break;
    }
    case OptModTimeSort :
    {
      modTimeSort = true;
      sizeSort = false;
      break;
    }
    case OptNoGroup     :
    {
      noGroup = true;
      break;
    }
    case OptSizeName    :
    {
      sizeName = true;
      break;
    }
    case OptNoOwners    :
    {
      noOwners = true;
      break;
    }
    default             :
    {
      break;
    }
  }
}

DirectoryList::DirectoryList(ftp::Client& client,
                             ftp::ReadWriteable& socket,
                             const fs::Path& path,
                             const ListOptions& options,
                             int maxRecursion) :
  client(client),
  socket(socket),
  path(path),
  options(options),
  maxRecursion(maxRecursion)
{
}

void DirectoryList::SplitPath(const fs::Path& path, fs::Path& parent,
                              std::queue<std::string>& masks)
{ 
  try
  {
    if (fs::Status(fs::MakeReal(path)).IsRegularFile())
    {
      parent = path.Dirname();
      masks.push(path.Basename().ToString());
      return;
    }
  }
  catch (const util::SystemError&)
  { }

  typedef boost::tokenizer<boost::char_separator<char>>  tokenizer;
  static const char* wildcardChars = "*?[]";

  if (path.IsAbsolute()) parent = fs::Path("/");
  bool foundWildcards = false;

  boost::char_separator<char> sep("/");
  tokenizer toks(path.ToString(), sep);
  for (const auto& token : toks)
  {
    if (foundWildcards ||
       token.find_first_of(wildcardChars) != std::string::npos)
    {
      masks.push(token);
      foundWildcards = true;
    }
    else
    {
      parent /= token;
    }
  }
}

void DirectoryList::Readdir(const fs::VirtualPath& path, fs::DirEnumerator& dirEnum) const
{
  dirEnum.Readdir(client.User(), path, !options.NoOwners() && !options.SizeName());

  if (options.SizeSort())
  {
    if (options.Reverse())
      std::sort(dirEnum.begin(), dirEnum.end(), fs::DirEntrySizeGreater());
    else
      std::sort(dirEnum.begin(), dirEnum.end(), fs::DirEntrySizeLess());      
  }
  else if (options.ModTimeSort())
  {
    if (options.Reverse())
      std::sort(dirEnum.begin(), dirEnum.end(), fs::DirEntryModTimeGreater());
    else
      std::sort(dirEnum.begin(), dirEnum.end(), fs::DirEntryModTimeLess());
  }
  else
  {
    if (options.Reverse())
      std::sort(dirEnum.begin(), dirEnum.end(), fs::DirEntryPathGreater());
    else
      std::sort(dirEnum.begin(), dirEnum.end(), fs::DirEntryPathLess());      
  }
}

void DirectoryList::Output(const std::string& message) const
{
  socket.Write(message.c_str(), message.length());
}

void DirectoryList::ListPath(const fs::Path& path, std::queue<std::string> masks, int depth) const
{
  if (maxRecursion && depth > maxRecursion) return;

  fs::DirEnumerator dirEnum;
  try
  {
    Readdir(MakeVirtual(path), dirEnum);
  }
  catch (const util::SystemError& e)
  {
    // silent failure - gives empty directory list
    return;
  }

  if (depth > 1) Output("\r\n");
  
  std::ostringstream message;
  if (!path.IsEmpty() && depth > 1 && (options.Recursive() || !masks.empty()))
  {
    message << path << ":\r\n";
    Output(message.str());
    message.str("");
  }
  
  if (options.LongFormat())
  {
    message << "total "
            << std::floor(dirEnum.TotalBytes() / 1024)
            << "\r\n";
    Output(message.str());
    message.str("");
  }
  
  std::string mask;
  if (!masks.empty())
  {
    mask = masks.front();
    masks.pop();
  }

  if (masks.empty())
  {
    for (const auto& de : dirEnum)
    {
      const std::string& pathStr = de.Path().ToString();
      if (pathStr[0] == '.' && !options.All()) continue;
      if (!mask.empty() && fnmatch(mask.c_str(), pathStr.c_str(), 0)) continue;
      
      if (options.LongFormat())
      {
        if (options.SizeName())
        {
          message << std::left << std::setw(8) << de.Status().Size() << " "
                  << de.Path();
        }
        else
        {
          message << Permissions(de.Status()) << " "
                  << std::setw(3) << de.Status().Native().st_nlink << " "
                  << std::left << std::setw(10) 
                  << UIDToName(de.Owner().UID()).substr(0, 10) << " ";
          
          if (!options.NoGroup())
          message << std::left << std::setw(10) 
                  << GIDToName(de.Owner().GID()).substr(0, 10) << " ";
                  
          message << std::right << std::setw(8) << de.Status().Size() << " "
                  << Timestamp(de.Status()) << " "
                  << de.Path();
        }
        
        if (options.SlashDirs() && de.Status().IsDirectory()) message << "/";
        message << "\r\n";
        Output(message.str());
        message.str("");
      }
      else
      {
        message << de.Path() << "\r\n";
        Output(message.str());
        message.str("");
      }
    }
  }
  
  if (options.Recursive() || !mask.empty())
  {
    for (const auto& de : dirEnum)
    {
      if (!de.Status().IsDirectory() ||
           de.Status().IsSymLink()) continue;
           
      const std::string& pathStr = de.Path().ToString();      
      if (pathStr[0] == '.' && !options.All()) continue;
      if (!mask.empty() && fnmatch(mask.c_str(), pathStr.c_str(), 0)) continue;

      fs::VirtualPath fullPath(path);
      fullPath /= de.Path();
   
      ListPath(fullPath, masks, depth + 1);
    }
  }
}

void DirectoryList::Execute()
{
  fs::Path parent;
  std::queue<std::string> masks;
  SplitPath(path, parent, masks);
  ListPath(parent, masks);
}

std::string DirectoryList::Permissions(const fs::Status& status)
{
  std::string perms(10, '-');
  
  if (status.IsDirectory()) perms[0] = 'd';
  else if (status.IsSymLink()) perms[0] = 'l';
  
  const struct stat& native = status.Native();
  
  if (native.st_mode & S_IRUSR) perms[1] = 'r';
  if (native.st_mode & S_IWUSR) perms[2] = 'w';
  if (native.st_mode & S_IXUSR) perms[3] = 'x';
  if (native.st_mode & S_IRGRP) perms[4] = 'r';
  if (native.st_mode & S_IWGRP) perms[5] = 'w';
  if (native.st_mode & S_IXGRP) perms[6] = 'x';
  if (native.st_mode & S_IROTH) perms[7] = 'r';
  if (native.st_mode & S_IWOTH) perms[8] = 'w';
  if (native.st_mode & S_IXOTH) perms[9] = 'x';
  
  return perms;
}

std::string DirectoryList::Timestamp(const fs::Status& status)
{
  char buf[13];
  strftime(buf, sizeof(buf),  "%b %d %H:%M", localtime(&status.Native().st_mtime));
  return buf;
}

const std::string& DirectoryList::UIDToName(acl::UserID uid) const
{
  std::unordered_map<acl::UserID, std::string>::const_iterator it =
    userNameCache.find(uid);
  if (it != userNameCache.end()) return it->second;
  return userNameCache[uid] = acl::UserCache::UIDToName(uid);
}

const std::string& DirectoryList::GIDToName(acl::GroupID gid) const
{
  std::unordered_map<acl::GroupID, std::string>::const_iterator it =
    groupNameCache.find(gid);
  if (it != groupNameCache.end()) return it->second;
  return groupNameCache[gid] = acl::GroupCache::GIDToName(gid);
}

} /* cmd namespace */
