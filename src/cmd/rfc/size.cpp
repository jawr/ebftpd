#include <boost/lexical_cast.hpp>
#include "cmd/rfc/size.hpp"
#include "fs/file.hpp"
#include "acl/path.hpp"
#include "cfg/get.hpp"

namespace cmd { namespace rfc
{

void SIZECommand::Execute()
{
  namespace PP = acl::path;

  fs::Path absolute = (client.WorkDir() / argStr).Expand();

  util::Error e(PP::FileAllowed<PP::View>(client.User(), absolute));
  if (!e)
  {
    control.Reply(ftp::ActionNotOkay, "SIZE failed 2: " + e.Message());
    return;
  }
  
  try
  {
    control.Reply(ftp::FileStatus, 
      boost::lexical_cast<std::string>(fs::SizeFile(client, argStr))); 
  }
  catch (const util::SystemError& e)
  {
    control.Reply(ftp::ActionNotOkay, "SIZE failed 1: " + e.Message());
    return;
  }
  
  return;
}

} /* rfc namespace */
} /* cmd namespace */
