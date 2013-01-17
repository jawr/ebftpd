#ifndef __EXEC_CSCRIPT_HPP
#define __EXEC_CSCRIPT_HPP

#include <string>

namespace ftp
{

class Client;

}

namespace exec
{

typedef cfg::setting::Cscript::Type CscriptType;

bool Cscripts(ftp::Client& client, const std::string& command, 
    const std::string& fullCommand, CscriptType type, ftp::ReplyCode failCode);

} /* exec namespace */

#endif

