#ifndef __PLUGIN_CLIENT_HPP
#define __PLUGIN_CLIENT_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "ftp/client.hpp"
#include "plugin/util.hpp"
#include "ftp/control.hpp"
#include "acl/user.hpp"
#include "plugin/user.hpp"
#include "plugin/locks.hpp"
#include "plugin/error.hpp"

namespace plugin
{

class Client
{
  ftp::Client* client;
  plugin::User user;
  
public:
  Client() { throw NotConstructable(); }
  explicit Client(ftp::Client& client) :
    client(&client),
    user(client.User())
  { }
  
  const boost::posix_time::ptime& LoggedInAt() const
  {
    return client->LoggedInAt();
  }

  boost::posix_time::seconds IdleTime() const
  {
    return client->IdleTime();
  }

  const boost::posix_time::seconds& IdleTimeout() const
  {
    return client->IdleTimeout();
  }

  void SetIdleTimeout(const boost::posix_time::seconds& idleTimeout)
  {
    client->SetIdleTimeout(idleTimeout);
  }

  ftp::ClientState State() const
  {
    return client->State();
  }

  boost::optional<plugin::User&> User()
  {
    if (client->State() != ftp::ClientState::LoggedIn) return boost::none;
    return boost::optional<plugin::User&>(user);
  }

  boost::optional<const plugin::User&> User() const
  {
    if (client->State() != ftp::ClientState::LoggedIn) return boost::none;
    return boost::optional<const plugin::User&>(user);
  }

  ftp::xdupe::Mode XDupeMode() const
  {
    return client->XDupeMode();
  }

  void SetXDupeMode(ftp::xdupe::Mode xdupeMode)
  {
    client->SetXDupeMode(xdupeMode);
  }

  std::string IP() const
  {
    return client->IP();
  }

  std::string Ident() const
  {
    return client->Ident();
  }

  std::string Hostname() const
  {
    return client->Hostname();
  }

  void SetSingleLineReplies(bool singleLineReplies)
  {
    client->Control().SetSingleLineReplies(singleLineReplies);
  }

  bool SingleLineReplies() const
  {
    return client->Control().SingleLineReplies();
  }

  void Reply(ftp::ReplyCode code, const std::string& message)
  {
    UnlockGuard unlock; (void) unlock;
    client->Control().Reply(code, message);
  }

  void PartReply(ftp::ReplyCode code, const std::string& message)
  {
    UnlockGuard unlock; (void) unlock;
    client->Control().PartReply(code, message);
  }
};

} /* script namespace */

#endif
