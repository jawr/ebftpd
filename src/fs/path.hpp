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

#ifndef __FS_PATH_HPP
#define __FS_PATH_HPP

#include <cassert>
#include <string>
#include <cstring>
#include <ostream>
#include "util/path/path.hpp"

namespace fs
{

class Path;
class VirtualPath;
class RealPath;

class Path
{
protected:
  struct Cache
  {
    VirtualPath* virt;
    RealPath* real;
    Path* dirname;
    Path* basename;
    
    Cache() : 
      virt(nullptr), real(nullptr), 
      dirname(nullptr), basename(nullptr) 
    { }

    Cache(const Cache&) : 
      virt(nullptr), real(nullptr), 
      dirname(nullptr), basename(nullptr) 
    { }
    
    Cache(Cache&& other) :
      virt(other.virt), real(other.real),
      dirname(other.dirname), basename(other.basename)
    {
      other.virt = nullptr;
      other.real = nullptr;
      other.dirname = nullptr;
      other.basename = nullptr;
    }
    
    void Clear();

    Cache& operator=(const Cache&)
    {
      Clear();
      return *this;
    }
    
    Cache& operator=(Cache&& other)
    {
      virt = other.virt;
      other.virt = nullptr;
      real = other.real;
      other.real = nullptr;
      dirname = other.dirname;
      other.dirname = nullptr;
      basename = other.basename;
      other.basename = nullptr;
      return *this;
    }
  };

  std::string path;
  mutable Cache cache;

  Path& operator=(const VirtualPath&) = delete;
  Path(const VirtualPath&) = delete;

  Path& operator=(const RealPath&) = delete;
  Path(const RealPath&) = delete;
  
  template <typename PathType>
  static PathType& Join(PathType& p1, const Path& p2)
  {
    p1.cache.Clear();
    p1.path = util::path::Join(p1.path, p2.path);
    return p1;
  }
  
  template <typename PathType>
  static PathType& Append(PathType& p1, const Path& p2)
  {
    p1.cache.Clear();
    p1.path = util::path::Append(p1.path, p2.path);
    return p1;
  }
  
  template <typename PathType>
  static const PathType& Dirname(const PathType& path)
  {
    if (!path.cache.dirname) 
      path.cache.dirname = new PathType(util::path::Dirname(path.ToString()));
    return *static_cast<PathType*>(path.cache.dirname);
  }
    
public:
  Path() = default;
  explicit Path(const std::string& path) : path(path) { }    
  explicit Path(const char* path) : path(path) { }  

  virtual ~Path() { cache.Clear(); }
    
  const Path& Dirname() const { return Dirname(*this); }  
  const Path& Basename() const
  {
    if (!cache.basename) 
      cache.basename = new Path(util::path::Basename(path));
    return *cache.basename;
  } 
  
  std::string Extension() const { return util::path::Extension(path); }
  std::string NoExtension() const { return util::path::NoExtension(path); }

  bool IsAbsolute() const { return !path.empty() && path[0] == '/'; }
  bool IsEmpty() const { return path.empty(); }
  void Clear() { path.clear(); }
  size_t Length() const { return path.length(); }

  const char* CString() const { return path.c_str(); }
  const std::string& ToString() const { return path; }
  
  virtual Path& operator/=(const Path& rhs) { return Join(*this, rhs); }
  virtual Path& operator/=(const std::string& rhs) { return Join(*this, Path(rhs)); }
  virtual Path& operator/=(const char* rhs) { return Join(*this, Path(rhs)); }
  
  virtual Path& operator&=(const Path& rhs) { return Append(*this, rhs); }
  virtual Path& operator&=(const std::string& rhs) { return Append(*this, Path(rhs)); }
  virtual Path& operator&=(const char* rhs) { return Append(*this, Path(rhs)); }

  bool operator==(const std::string& rhs) const { return path == rhs; }
  bool operator!=(const std::string& rhs) const { return !operator==(rhs); }

  bool operator==(const char* rhs) const { return !path.compare(rhs); }
  bool operator!=(const char* rhs) const { return !operator==(rhs); }

  bool operator==(const Path& rhs) const { return path == rhs.path; }
  bool operator!=(const Path& rhs) const { return !operator==(rhs); }
  
  bool operator<(const Path& rhs) const 
  { return strcasecmp(path.c_str(), rhs.path.c_str()) < 0; }
  
  bool operator>(const Path& rhs) const
  { return strcasecmp(path.c_str(), rhs.path.c_str()) > 0; }
  
  // used for modifying the cache
  friend const VirtualPath& MakeVirtual(const Path& path);
  friend const RealPath& MakeReal(const Path& path);
};

class VirtualPath : public Path
{
public:
  VirtualPath() = default;
  explicit VirtualPath(const std::string& path) : Path(path) { }
  explicit VirtualPath(const char* path) : Path(path) { }
  explicit VirtualPath(const RealPath&) = delete;
  explicit VirtualPath(const Path& path) : Path(path) { }
  
  const VirtualPath& Dirname() const { return Path::Dirname(*this); }  
  
  VirtualPath& operator/=(const RealPath&) = delete;
  virtual VirtualPath& operator/=(const Path& rhs) { return Join(*this, rhs); }
  virtual VirtualPath& operator/=(const std::string& rhs) { return Join(*this, Path(rhs)); }
  virtual VirtualPath& operator/=(const char* rhs) { return Join(*this, Path(rhs)); }

  VirtualPath& operator&=(const RealPath&) = delete;
  virtual VirtualPath& operator&=(const Path& rhs) { return Append(*this, rhs); }
  virtual VirtualPath& operator&=(const std::string& rhs) { return Append(*this, Path(rhs)); }
  virtual VirtualPath& operator&=(const char* rhs) { return Append(*this, Path(rhs)); }

  // used for modifying the cache
  friend const RealPath& MakeReal(const VirtualPath& path);
};

class RealPath : public Path
{
public:
  RealPath() = default;
  explicit RealPath(const std::string& path) : Path(path) { }
  explicit RealPath(const char* path) : Path(path) { }
  explicit RealPath(const VirtualPath&) = delete;
  explicit RealPath(const Path& path) : Path(path) { }
  
  const RealPath& Dirname() const { return Path::Dirname(*this); }  

  RealPath& operator/=(const VirtualPath&) = delete;
  virtual RealPath& operator/=(const Path& rhs) { return Join(*this, rhs); }
  virtual RealPath& operator/=(const std::string& rhs) { return Join(*this, Path(rhs)); }
  virtual RealPath& operator/=(const char* rhs) { return Join(*this, Path(rhs)); }

  RealPath& operator&=(const RealPath&) = delete;
  virtual RealPath& operator&=(const Path& rhs) { return Append(*this, rhs); }  
  virtual RealPath& operator&=(const std::string& rhs) { return Append(*this, Path(rhs)); }  
  virtual RealPath& operator&=(const char* rhs) { return Append(*this, Path(rhs)); }  
  
  // used for modifying the cache
  friend const VirtualPath& MakeVirtual(const RealPath& path);
};

inline void Path::Cache::Clear()
{
  delete virt;
  virt = nullptr;
  delete real;
  real = nullptr;
  delete dirname;
  dirname = nullptr;
  delete basename;
  basename = nullptr;
}

inline Path operator/(const Path& lhs, const Path& rhs)
{ return Path(lhs) /= rhs; }

inline Path operator/(const Path& lhs, const std::string& rhs)
{ return Path(lhs) /= rhs; }

inline Path operator/(const std::string& lhs, const Path& rhs)
{ return Path(lhs) /= rhs; }

inline Path operator/(const char* lhs, const Path& rhs)
{ return Path(lhs) /= rhs; }

inline Path operator&(const Path& lhs, const Path& rhs)
{ return Path(lhs) &= rhs; }

inline Path operator&(const Path& lhs, const std::string& rhs)
{ return Path(lhs) &= rhs; }

inline Path operator&(const std::string& lhs, Path& rhs)
{ return Path(lhs) &= rhs; }

inline Path operator&(const Path& lhs, const char* rhs)
{ return Path(lhs) &= rhs; }

inline Path operator&(const char* lhs, const Path& rhs)
{ return Path(lhs) &= rhs; }

inline VirtualPath operator/(const VirtualPath& lhs, const Path& rhs)
{ return VirtualPath(lhs) /= rhs; }

inline VirtualPath operator/(const VirtualPath& lhs, const std::string& rhs)
{ return VirtualPath(lhs) /= rhs; }

inline VirtualPath operator/(const std::string& lhs, const VirtualPath& rhs)
{ return VirtualPath(lhs) /= rhs; }

inline VirtualPath operator/(const VirtualPath& lhs, const char* rhs)
{ return VirtualPath(lhs) /= rhs; }

inline VirtualPath operator/(const char* lhs, const VirtualPath& rhs)
{ return VirtualPath(lhs) /= rhs; }

inline VirtualPath operator&(const VirtualPath& lhs, const Path& rhs)
{ return VirtualPath(lhs) &= rhs; }

inline VirtualPath operator&(const VirtualPath& lhs, const std::string& rhs)
{ return VirtualPath(lhs) &= rhs; }

inline VirtualPath operator&(const VirtualPath& lhs, const char* rhs)
{ return VirtualPath(lhs) &= rhs; }

inline RealPath operator/(const RealPath& lhs, const Path& rhs)
{ return RealPath(lhs) /= rhs; }

inline RealPath operator/(const RealPath& lhs, const std::string& rhs)
{ return RealPath(lhs) /= rhs; }

inline RealPath operator/(const std::string& lhs, const RealPath& rhs)
{ return RealPath(lhs) /= rhs; }

inline RealPath operator/(const RealPath& lhs, const char* rhs)
{ return RealPath(lhs) /= rhs; }

inline RealPath operator/(const char* lhs, const RealPath& rhs)
{ return RealPath(lhs) /= rhs; }

inline RealPath operator&(const RealPath& lhs, const Path& rhs)
{ return RealPath(lhs) &= rhs; }

inline RealPath operator&(const RealPath& lhs, const std::string& rhs)
{ return RealPath(lhs) &= rhs; }

inline RealPath operator&(const RealPath& lhs, const char* rhs)
{ return RealPath(lhs) &= rhs; }

inline std::ostream& operator<<(std::ostream& os, const Path& path)
{ return (os << path.ToString()); }

template <typename PathType>
inline PathType Resolve(const PathType& path)
{ return PathType(util::path::Resolve(path.ToString())); }

Path MakeRelative(const VirtualPath& path);
Path MakePretty(const VirtualPath& path);
Path MakePretty(const Path& path);

const VirtualPath& MakeVirtual(const Path& path);
const VirtualPath& MakeVirtual(const VirtualPath& path);
const VirtualPath& MakeVirtual(const RealPath& path);

const RealPath& MakeReal(const Path& path);
const RealPath& MakeReal(const VirtualPath& path);
const RealPath& MakeReal(const RealPath& path);

VirtualPath PathFromUser(const std::string& path);

} /* fs namespace */

#endif
