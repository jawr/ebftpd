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

#include <stdexcept>
#include "fs/path.hpp"
#include "cfg/get.hpp"
#include "logs/logs.hpp"
#include "fs/directory.hpp"

namespace fs
{

Path MakeRelative(const VirtualPath& path)
{
  return Path(util::path::Relative(WorkDirectory().ToString(), path.ToString()));
}

Path MakePretty(const VirtualPath& path)
{
  Path pretty(MakeRelative(path));
  if (pretty.IsEmpty()) return Path(".");
  if (pretty.ToString().compare(0, 3, "../")) return pretty;
  return Path(path.ToString());
}

Path MakePretty(const Path& path)
{
  if (path.IsAbsolute()) return path;
  return MakePretty(MakeVirtual(path));
}

const VirtualPath& MakeVirtual(const Path& path)
{
  if (!path.cache.virt)
  {
    return MakeVirtual(MakeReal(path));
  }
  return *path.cache.virt;
}

const VirtualPath& MakeVirtual(const VirtualPath& path)
{
  return path;
}

const VirtualPath& MakeVirtual(const RealPath& path)
{
  if (!path.cache.virt)
  {
    const std::string& sitepath = cfg::Get().Sitepath();
    const std::string& pathstr = path.ToString();
    if (!pathstr.compare(0, sitepath.length(), sitepath) &&
        (pathstr[sitepath.length()] == '/' ||
         pathstr.length() == sitepath.length()))
    {
      std::string newpath(path.ToString(), cfg::Get().Sitepath().length());
      if (newpath.empty()) newpath = "/";
      path.cache.virt = new VirtualPath(newpath);
    }
    else
    {
      // if the rest of this code is correct, we should never get here
      throw std::logic_error("Path resides outside the site root");
    }
  }
  return *path.cache.virt;
}

const RealPath& MakeReal(const Path& path)
{
  if (!path.cache.real)
  {
    auto virt = Resolve(WorkDirectory() / path);
    path.cache.real = new RealPath(RealPath(cfg::Get().Sitepath()) & virt);
    std::string& p = path.cache.real->path;
    std::string noSymlinks;
    if (util::path::Realpath(p, noSymlinks))
    {
      p = noSymlinks;
    }
    else
    {
      // deal with paths that don't exist yet
      if (util::path::Realpath(util::path::Dirname(p), noSymlinks))
      {
        p = util::path::Join(noSymlinks, util::path::Basename(p));
      }
    }
  }
  return *path.cache.real;
}

const RealPath& MakeReal(const VirtualPath& path)
{
  if (!path.cache.real)
    path.cache.real = new RealPath(RealPath(cfg::Get().Sitepath()) & path);
  return *path.cache.real;
}

const RealPath& MakeReal(const RealPath& path)
{
  return path;
}

VirtualPath PathFromUser(const std::string& path)
{
  return MakeVirtual(Path(path));
}


} /* fs namespace */
