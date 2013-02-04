#include <boost/lexical_cast.hpp>
#include "cmd/rfc/size.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"
#include "fs/status.hpp"

namespace cmd { namespace rfc
{

void SIZECommand::Execute()
{
  namespace PP = acl::path;

  fs::VirtualPath path(fs::PathFromUser(argStr));

  try
  {
    fs::Status status(client.User(), path);
    if (status.IsRegularFile())
      control.Reply(ftp::FileStatus, 
        boost::lexical_cast<std::string>(status.Size())); 
    else
      control.Reply(ftp::ActionNotOkay, argStr + ": Not a plain file.");
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, argStr + ": " + e.Message());
    return;
  }
  
  return;
}

} /* rfc namespace */
} /* cmd namespace */
