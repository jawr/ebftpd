#ifndef __EXEC_CSCRIPT_HPP
#define __EXEC_CSCRIPT_HPP

#include <string>
#include "cfg/get.hpp"
#include "ftp/replycodes.hpp"

namespace ftp
{

class Client;

}

namespace exec
{

typedef cfg::Cscript::Type CscriptType;

bool Cscripts(ftp::Client& client, const std::string& command, 
    const std::string& fullCommand, CscriptType type, ftp::ReplyCode failCode);

} /* exec namespace */

#endif

