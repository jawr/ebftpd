#ifndef __FS_FILE_HPP
#define __FS_FILE_HPP

#include <tr1/memory>
#include <string>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include "util/error.hpp"

namespace ftp
{
class Client;
}

namespace fs
{
typedef boost::iostreams::stream<
        boost::iostreams::file_descriptor_sink> OutStream;
        
typedef std::tr1::shared_ptr<OutStream> OutStreamPtr;

typedef boost::iostreams::stream<
        boost::iostreams::file_descriptor_source> InStream;
        
typedef std::tr1::shared_ptr<InStream> InStreamPtr;
        
util::Error DeleteFile(const ftp::Client& client, const std::string& path);
util::Error RenameFile(const ftp::Client& client, const std::string& oldPath,
                       const std::string& newPath);
OutStreamPtr CreateFile(const ftp::Client& client, const std::string& path);
OutStreamPtr AppendFile(const ftp::Client& client, const std::string& path);
InStreamPtr OpenFile(const ftp::Client& client, const std::string& path);

} /* fs namespace */

#endif
