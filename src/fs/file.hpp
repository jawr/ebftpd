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
        
util::Error DeleteFile(ftp::Client& client, const Path& path, off_t* size = 0);
util::Error ForceDeleteFile(ftp::Client& client, const Path& path);
util::Error RenameFile(ftp::Client& client, const Path& oldPath,
                       const Path& newPath);
FileSinkPtr CreateFile(ftp::Client& client, const Path& path);
FileSinkPtr AppendFile(ftp::Client& client, const Path& path, off_t offset);
FileSourcePtr OpenFile(ftp::Client& client, const Path& path);
util::Error UniqueFile(ftp::Client& client, const Path& path, 
                       size_t filenameLength, Path& uniquePath);

off_t SizeFile(ftp::Client& client, const Path& path); // throws an exception and doesn't check perms
bool IsIncomplete(ftp::Client& client, const Path& path);


} /* fs namespace */

#endif
