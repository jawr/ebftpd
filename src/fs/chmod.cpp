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

#include <algorithm>
#include <cctype>
#include <bitset>
#include <sys/stat.h>
#include "fs/chmod.hpp"
#include "acl/user.hpp"
#include "fs/mode.hpp"
#include "util/path/status.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/path.hpp"

namespace fs
{

util::Error Chmod(const RealPath& path, const Mode& mode)
{
  try
  {
    mode_t newMode;
    mode.Apply(util::path::Status(path.ToString()).Native().st_mode, umask(0), newMode);
  
    if (chmod(MakeReal(path).CString(), newMode) < 0)
      return util::Error::Failure(errno);
  }
  catch (const util::SystemError& e)
  { return util::Error::Failure(e.Errno()); }
  
  return util::Error::Success();
}

util::Error Chmod(const acl::User& user, const VirtualPath& path, const Mode& mode)
{
  namespace PP = acl::path;

  util::Error e = PP::FileAllowed<PP::View>(user, path);
  if (!e) return e;
  
  mode_t userMask = umask(0);
  
  try
  {
    util::path::Status status(MakeReal(path).ToString());
    if (status.IsDirectory())
    {
      util::Error e = PP::DirAllowed<PP::View>(user, path);
      if (!e) return e;
    }
    
    mode_t newMode;
    mode.Apply(status.Native().st_mode, userMask, newMode);
    
    if (chmod(MakeReal(path).CString(), newMode) < 0)
      return util::Error::Failure(errno);
  }
  catch (const util::SystemError& e)
  {
    return util::Error::Failure(e.Errno());
  }
  
  return util::Error::Success();
}

} /* fs namespace */
