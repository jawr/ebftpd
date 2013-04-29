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

#ifndef __FS_FILE_HPP
#define __FS_FILE_HPP

#include <memory>
#include <string>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <sys/types.h>

namespace acl
{
class User;
}

namespace util
{
class Error;
}

namespace fs
{

class RealPath;
class VirtualPath;

typedef boost::iostreams::file_descriptor_sink FileSink;        
typedef std::shared_ptr<FileSink> FileSinkPtr;
typedef boost::iostreams::file_descriptor_source FileSource;
typedef std::shared_ptr<FileSource> FileSourcePtr;

util::Error DeleteFile(const RealPath& path);
util::Error DeleteFile(const acl::User& user, const VirtualPath& path, 
                       off_t* size = nullptr, time_t* modTime = nullptr);

util::Error Rename(const RealPath& oldPath, const RealPath& newPath);

FileSinkPtr CreateFile(const acl::User& user, const VirtualPath& path);
FileSinkPtr AppendFile(const acl::User& user, const VirtualPath& path, off_t offset);
FileSourcePtr OpenFile(const acl::User& user, const VirtualPath& path);
util::Error UniqueFile(const acl::User& user, const VirtualPath& path, 
                       size_t filenameLength, VirtualPath& uniquePath);

bool IsIncomplete(const RealPath& path);

} /* fs namespace */

#endif
