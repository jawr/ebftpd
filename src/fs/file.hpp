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

typedef boost::iostreams::stream<
        boost::iostreams::file_descriptor_sink> OutStream;
        
typedef std::shared_ptr<OutStream> OutStreamPtr;

typedef boost::iostreams::stream<
        boost::iostreams::file_descriptor_source> InStream;
        
typedef std::shared_ptr<InStream> InStreamPtr;
        
util::Error DeleteFile(ftp::Client& client, const Path& path, off_t* size = 0);
util::Error RenameFile(ftp::Client& client, const Path& oldPath,
                       const Path& newPath);
OutStreamPtr CreateFile(ftp::Client& client, const Path& path);
OutStreamPtr AppendFile(ftp::Client& client, const Path& path);
InStreamPtr OpenFile(ftp::Client& client, const Path& path);
util::Error UniqueFile(ftp::Client& client, const Path& path, 
                       size_t filenameLength, Path& uniquePath);

off_t SizeFile(ftp::Client& client, const Path& path); // throws an exception and doesn't check perms

} /* fs namespace */

#endif
