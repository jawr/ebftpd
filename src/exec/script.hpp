#ifndef __EXEC_SCRIPT_HPP
#define __EXEC_SCRIPT_HPP

#include "reader.hpp"
#include "util/processreader.hpp"

namespace fs
{
class Path;
}

namespace ftp
{
class Client;
}

namespace exec
{

int Script(ftp::Client& client, const util::ProcessReader::ArgvType& argv, 
        std::string& messages);

}

#endif
