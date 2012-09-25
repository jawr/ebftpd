#ifndef __CMD_DIRLIST_HPP
#define __CMD_DIRLIST_HPP

#include <string>
#include <queue>
#include "fs/path.hpp"
#include "fs/status.hpp"
#include "util/net/tcpsocket.hpp"

namespace ftp
{
class Client;
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
};

class DirectoryList
{
  ftp::Client& client;
  util::net::TCPSocket& socket;
  fs::Path path;
  ListOptions options;
  bool dataOutput;
  fs::Path parent;
  std::queue<std::string> masks;
  
  void SplitPath();
  void ListPath(const fs::Path& path, std::queue<std::string> masks) const;
  void Readdir(const fs::Path& path, fs::DirEnumerator& dirEnum) const;
  void Output(const std::string& message) const;
  
  static std::string Permissions(const fs::Status& status);
  static std::string Timestamp(const fs::Status& status);
  
public:
  DirectoryList(ftp::Client& client, const fs::Path& path,
                const ListOptions& options,
                bool dataOutput);
                
  void Execute();
};

} /* cmd namespace */

#endif
