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

#include <iomanip>
#include <cmath>
#include <memory>
#include <cstring>
#include <algorithm>
#include <dirent.h>
#include <fnmatch.h>
#include <boost/tokenizer.hpp>
#include "cmd/rfc/dirlist.hpp"
#include "ftp/client.hpp"
#include "fs/direnumerator.hpp"
#include "util/path/status.hpp"
#include "logs/logs.hpp"
#include "acl/user.hpp"
#include "acl/group.hpp"
#include "ftp/data.hpp"
#include "cfg/get.hpp"
#include "stats/util.hpp"
#include "util/scopeguard.hpp"

namespace cmd { namespace rfc
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
                             ftp::Writeable& socket,
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

void DirectoryList::SplitPath(const fs::Path& path, fs::VirtualPath& parent,
                              std::queue<std::string>& masks)
{ 
  try
  {
    if (util::path::Status(fs::MakeReal(path).ToString()).IsRegularFile())
    {
      parent = fs::PathFromUser(path.Dirname().ToString());
      masks.push(path.Basename().ToString());
      return;
    }
  }
  catch (const util::SystemError&)
  { }

  typedef boost::tokenizer<boost::char_separator<char>>  tokenizer;
  static const char* wildcardChars = "*?[]";

  if (path.IsAbsolute()) parent = fs::PathFromUser("/");
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
  
  parent = fs::PathFromUser(parent.ToString());
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

void DirectoryList::ListPath(const fs::VirtualPath& path, std::queue<std::string> masks, int depth) const
{
  if (maxRecursion && depth > maxRecursion) return;

  fs::DirEnumerator dirEnum;
  try
  {
    Readdir(path, dirEnum);
  }
  catch (const util::SystemError& e)
  {
    // silent failure - gives empty directory list
    return;
  }
  

  std::ostringstream message;
  if (depth > 1) message << "\r\n";
  
  if (!path.IsEmpty() && depth > 1 && (options.Recursive() || !masks.empty()))
  {
    message << path << ":\r\n";
  }
  
  if (options.LongFormat())
  {
    message << "total " << static_cast<long long>(dirEnum.TotalBytes() / 1024) << "\r\n";
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
          message << std::left << std::setw(10) << de.Status().Size() << ' '
                  << de.Path();
        }
        else
        {
          message << Permissions(de.Status()) << ' '
                  << std::setw(3) << de.Status().Native().st_nlink << ' '
                  << std::left << std::setw(10) 
                  << UIDToName(de.Owner().UID()) << ' ';
          
          if (!options.NoGroup())
            message << std::left << std::setw(10) 
                    << GIDToName(de.Owner().GID()) << ' ';
                  
          message << std::right << std::setw(10) << de.Status().Size() << ' '
                  << Timestamp(de.Status()) << ' '
                  << de.Path();
        }
        
        if (de.Status().IsSymLink())
        {
          auto real = fs::MakeReal(path / de.Path());
          std::string dest;
          if (util::path::Readlink(real.ToString(), dest))
          {
            message << " -> " << dest;
          }
        }
                
        if (options.SlashDirs() && de.Status().IsDirectory()) message << '/';
        message << "\r\n";
      }
      else
      {
        message << de.Path() << "\r\n";
      }
    }
  }
  
  Output(message.str());
  
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
  fs::VirtualPath parent;
  std::queue<std::string> masks;
  SplitPath(path, parent, masks);
  ListPath(parent, masks);
}

std::string DirectoryList::Permissions(const util::path::Status& status)
{
  std::string perms(10, '-');
  
  if (status.IsSymLink()) perms[0] = 'l';
  else if (status.IsDirectory()) perms[0] = 'd';
  
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


std::string DirectoryList::Timestamp(const util::path::Status& status) const
{
  time_t modTime = status.Native().st_mtime - status.Native().st_mtime % 60;
  auto it = timestampCache.find(modTime);
  if (it != timestampCache.end()) return it->second;
  char buf[13];
  strftime(buf, sizeof(buf), "%b %d %H:%M", localtime(&modTime));
  return timestampCache[modTime] = buf;
}
  
const std::string& DirectoryList::UIDToName(acl::UserID uid) const
{
  auto it = userNameCache.find(uid);
  if (it != userNameCache.end()) return it->second;
  return userNameCache[uid] = acl::UIDToName(uid).substr(0, 10);
}

const std::string& DirectoryList::GIDToName(acl::GroupID gid) const
{
  auto it = groupNameCache.find(gid);
  if (it != groupNameCache.end()) return it->second;
  return groupNameCache[gid] = acl::GIDToName(gid).substr(0, 10);
}

void LISTCommand::ExecuteNLST()
{
  nlst = true;
  Execute();
}

void LISTCommand::Execute()
{
  std::ostringstream os;
  os << "Opening connection for directory listing";
  if (data.Protection()) os << " using TLS/SSL";
  os << ".";
  control.Reply(ftp::TransferStatusOkay, os.str());

  try
  {
    data.Open(ftp::TransferType::List);
  }
  catch (const util::net::NetworkError&e )
  {
    control.Reply(ftp::CantOpenDataConnection,
                 "Unable to open data connection: " + e.Message());
    return;
  }
  if (!data.ProtectionOkay())
  {
    data.Close();
    control.Reply(ftp::ProtocolNotSupported, 
                  "TLS is enforced on directory listings.");
    return;
  }

  std::string options;
  fs::Path path;
  if (args.size() >= 2)
  {
    std::string::size_type optOffset = 0;
    if (args[1][0] == '-')
    {
      options = args[1].substr(1);
      optOffset += args[1].length();
    }
    
    path = fs::Path(util::TrimCopy(std::string(argStr, optOffset)));
  }
  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions(nlst ? "" : "l" + config.Lslong().Options());
  
  DirectoryList dirList(client, data, path, ListOptions(options, forcedOptions),
                        config.Lslong().MaxRecursion());

  try
  {
    dirList.Execute();    
  }
  catch (const util::net::NetworkError& e)
  {
    data.Close();
    control.Reply(ftp::DataCloseAborted,
                "Error whiling writing to data connection: " + e.Message());
    return;
  }
  
  data.Close();
  control.Reply(ftp::DataClosedOkay, "End of directory listing (" + 
      stats::HighResSecondsString(data.State().StartTime(), data.State().EndTime()) + ")"); 
}

void NLSTCommand::Execute()
{
  LISTCommand(client, argStr, args).ExecuteNLST();
}

void STATCommand::Execute()
{
  if (args.size() == 1)
  {
    std::ostringstream os;
    os << programFullname << " status\n";
    os << "< Insert status info here >\n";
    os << "End of status.";
    control.Reply(ftp::SystemStatus, os.str());
    return;
  }
  
  bool singleLineReplies = control.SingleLineReplies();
  control.SetSingleLineReplies(false);
  
  auto singleLineGuard = util::MakeScopeExit([&]{ control.SetSingleLineReplies(singleLineReplies); });  

  std::string options;
  std::string::size_type optOffset = 0;
  if (args[1][0] == '-')
  {
    options = args[1].substr(1);
    optOffset += args[1].length();
  }
  
  fs::Path path(util::TrimCopy(std::string(argStr, optOffset)));
  
  const cfg::Config& config = cfg::Get();
  std::string forcedOptions = "l" + config.Lslong().Options();
    
  control.PartReply(ftp::DirectoryStatus, "Status of " + fs::MakePretty(MakeVirtual(path)).ToString() + ":");
  DirectoryList dirList(client, control, path, ListOptions(options, forcedOptions),
                        config.Lslong().MaxRecursion());
  
  boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
  dirList.Execute();
  boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
  
  control.Reply(ftp::DirectoryStatus, "End of status (" + stats::HighResSecondsString(start, end) + ")"); 
  return;
  
  (void) singleLineReplies;
  (void) singleLineGuard;
}

} /* rfc namespace */  
} /* cmd namespace */
