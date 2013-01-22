#ifndef __FS_FILE_HPP
#define __FS_FILE_HPP

#include <memory>
#include <string>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <sys/types.h>
#include "util/error.hpp"
#include "fs/path.hpp"

namespace ftp
{
class Client;
}

namespace fs
{

typedef boost::iostreams::file_descriptor_sink FileSink;        
typedef std::shared_ptr<FileSink> FileSinkPtr;
typedef boost::iostreams::file_descriptor_source FileSource;
typedef std::shared_ptr<FileSource> FileSourcePtr;

util::Error DeleteFile(const RealPath& path);
util::Error DeleteFile(ftp::Client& client, const VirtualPath& path, 
                       off_t* size = nullptr, time_t* modTime = nullptr);

util::Error RenameFile(const RealPath& oldPath, const RealPath& newPath);
util::Error RenameFile(ftp::Client& client, const VirtualPath& oldPath,
                       const VirtualPath& newPath);

FileSinkPtr CreateFile(ftp::Client& client, const VirtualPath& path);
FileSinkPtr AppendFile(ftp::Client& client, const VirtualPath& path, off_t offset);
FileSourcePtr OpenFile(ftp::Client& client, const VirtualPath& path);
util::Error UniqueFile(ftp::Client& client, const VirtualPath& path, 
                       size_t filenameLength, VirtualPath& uniquePath);

bool IsIncomplete(const RealPath& path);

} /* fs namespace */

#endif
