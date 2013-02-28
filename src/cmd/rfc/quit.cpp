#include "cmd/rfc/quit.hpp"
#include "text/util.hpp"
#include "acl/misc.hpp"
#include "fs/path.hpp"

namespace cmd { namespace rfc
{

void QUITCommand::Execute()
{
  std::string goodbye;
  if (client.State() == ftp::ClientState::LoggedIn)
  {
    fs::Path goodbyePath(acl::message::Choose<acl::message::Goodbye>(client.User()));
    if (!goodbyePath.IsEmpty() &&
        text::GenericTemplate(client, goodbyePath, goodbye))
    {
      control.Reply(ftp::ClosingControl, goodbye);
    }
  }
  
  if (goodbye.empty()) control.Reply(ftp::ClosingControl, "Bye bye");

  client.SetState(ftp::ClientState::Finished);
}

} /* rfc namespace */
} /* cmd namespace */
