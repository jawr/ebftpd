#ifndef __CMD_DIRLIST_HPP
#define __CMD_DIRLIST_HPP

#include <string>
#include <queue>
#include <unordered_map>
#include "fs/path.hpp"
#include "fs/status.hpp"
#include "acl/types.hpp"

namespace ftp
{
class Client;
class ReadWriteable;
}

namespace fs
{
class DirEnumerator;
}

namespace cmd
{

class ListOptions
{
  bool all;
  bool longFormat;
  bool slashDirs;
  bool reverse;
  bool recursive;
  bool sizeSort;
  bool modTimeSort;
  bool noGroup;
  bool sizeName;
  bool noOwners;

  void ParseOption(char option);
  
public:
  ListOptions(const std::string& userDefined, const std::string& forced);
  
  bool All() const { return all; }
  bool LongFormat() const { return longFormat; }
  bool SlashDirs() const { return slashDirs; }
  bool Reverse() const { return reverse; }
  bool Recursive() const { return recursive; }
  bool SizeSort() const { return sizeSort; }
  bool ModTimeSort() const { return modTimeSort; }
  bool NoGroup() const { return noGroup; }
  bool SizeName() const { return sizeName; }
  bool NoOwners() const { return noOwners; }
};

class DirectoryList
{
  ftp::Client& client;
  ftp::ReadWriteable& socket;
  fs::Path path;
  ListOptions options;
  bool dataOutput;
  int maxRecursion;
  
  mutable std::unordered_map<acl::UserID, std::string> userNameCache;
  mutable std::unordered_map<acl::GroupID, std::string> groupNameCache;
  
  void ListPath(const fs::Path& path, std::queue<std::string> masks, int depth = 1) const;
  void Readdir(const fs::VirtualPath& path, fs::DirEnumerator& dirEnum) const;
  inline void Output(const std::string& message) const;
  
  inline const std::string& UIDToName(acl::UserID uid) const;
  inline const std::string& GIDToName(acl::GroupID gid) const;
  
  static void SplitPath(const fs::Path& path, fs::Path& parent,
                        std::queue<std::string>& masks);
                        
  inline static std::string Permissions(const fs::Status& status);
  inline static std::string Timestamp(const fs::Status& status);
  
public:
  DirectoryList(ftp::Client& client,
                ftp::ReadWriteable& socket,
                const fs::Path& path,
                const ListOptions& options,
                int maxRecursion);
                
  void Execute();
};

} /* cmd namespace */

#endif
