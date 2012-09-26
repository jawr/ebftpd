#ifndef __UTIL_FS_DIRENUMERATOR_HPP
#define __UTIL_FS_DIRENUMERATOR_HPP

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/cstdint.hpp>
#include "fs/path.hpp"
#include "fs/status.hpp"
#include "fs/owner.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

class DirEntry
{
  fs::Path path;
  fs::Status status;
  fs::Owner owner;
  
public:  
  DirEntry(const fs::Path& path, const fs::Status& status,
           const fs::Owner& owner) :
    path(path), status(status), owner(owner) { }

  const fs::Path& Path() const { return path; }
  const fs::Status& Status() const { return status; }
  const fs::Owner& Owner() const { return owner; }
};

class DirEnumerator
{
  ftp::Client* client;
  fs::Path path;
  uintmax_t totalBytes;
  
  boost::ptr_vector<DirEntry> entries;
  
  void Readdir();
  
public:
  typedef boost::ptr_vector<DirEntry>::const_iterator const_iterator;
  typedef boost::ptr_vector<DirEntry>::iterator iterator;
  typedef boost::ptr_vector<DirEntry>::size_type size_type;

  DirEnumerator();
  DirEnumerator(const fs::Path& path);
  DirEnumerator(ftp::Client& client, const fs::Path& path);
  
  void Readdir(const fs::Path& path);
  void Readdir(ftp::Client& client, const fs::Path& path);

  uintmax_t TotalBytes() const { return totalBytes; }
  
  const_iterator begin() const { return entries.begin(); }
  const_iterator end() const { return entries.end(); }
  iterator begin() { return entries.begin(); }
  iterator end() { return entries.end(); }
  size_type size() const { return entries.size(); }
  bool empty() const { return entries.empty(); }
};

struct DirEntryPathLess
{
  bool operator()(const DirEntry& de1, const DirEntry& de2)
  { return de1.Path() < de2.Path(); }  
};

struct DirEntryPathGreater
{
  bool operator()(const DirEntry& de1, const DirEntry& de2)
  { return de1.Path() > de2.Path(); }  
};

struct DirEntrySizeLess
{
  bool operator()(const DirEntry& de1, const DirEntry& de2)
  { return de1.Status().Size() < de2.Status().Size(); }  
};

struct DirEntrySizeGreater
{
  bool operator()(const DirEntry& de1, const DirEntry& de2)
  { return de1.Status().Size() > de2.Status().Size(); }  
};

struct DirEntryModTimeLess
{
  bool operator()(const DirEntry& de1, const DirEntry& de2)
  { return de1.Status().Native().st_mtime < de2.Status().Native().st_mtime; }  
};

struct DirEntryModTimeGreater
{
  bool operator()(const DirEntry& de1, const DirEntry& de2)
  { return de1.Status().Native().st_mtime > de2.Status().Native().st_mtime; }  
};

} /* fs namespace */

#endif
