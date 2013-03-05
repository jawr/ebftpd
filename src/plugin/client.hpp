#ifndef __PLUGIN_CLIENT_HPP
#define __PLUGIN_CLIENT_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "plugin/client.hpp"
#include "ftp/client.hpp"
#include "plugin/util.hpp"
#include "ftp/control.hpp"
#include "acl/user.hpp"
#include "plugin/user.hpp"
#include "plugin/unlockable.hpp"

namespace plugin
{

class Client : private Unlockable
{
  ftp::Client& client;
  
  Client& operator=(Client&&) = delete;
  Client& operator=(const Client&) = delete;
  Client(Client&&) = delete;
  Client(const Client&) = delete;
  
public:
  explicit Client(ftp::Client& client) : client(client) { }
  
  const boost::posix_time::ptime& LoggedInAt() const
  {
    return client.LoggedInAt();
  }

  boost::posix_time::seconds IdleTime() const
  {
    return client.IdleTime();
  }

  const boost::posix_time::seconds& IdleTimeout() const
  {
    return client.IdleTimeout();
  }

  void SetIdleTimeout(const boost::posix_time::seconds& idleTimeout)
  {
    client.SetIdleTimeout(idleTimeout);
  }

  ftp::ClientState State() const
  {
    return client.State();
  }

  boost::optional<plugin::User<NoUnlocking>> User()
  {
    if (client.State() != ftp::ClientState::LoggedIn) return boost::none;
    return boost::optional<plugin::User<NoUnlocking>>(plugin::User<NoUnlocking>(client.User()));
  }

  boost::optional<const plugin::User<NoUnlocking>> User() const
  {
    if (client.State() != ftp::ClientState::LoggedIn) return boost::none;
    return boost::optional<const plugin::User<NoUnlocking>>(plugin::User<NoUnlocking>(client.User()));
  }

  ftp::xdupe::Mode XDupeMode() const
  {
    return client.XDupeMode();
  }

  void SetXDupeMode(ftp::xdupe::Mode xdupeMode)
  {
    client.SetXDupeMode(xdupeMode);
  }

  std::string IP() const
  {
    return client.IP();
  }

  std::string Ident() const
  {
    return client.Ident();
  }

  std::string Hostname() const
  {
    return client.Hostname();
  }

  void SetSingleLineReplies(bool singleLineReplies)
  {
    client.Control().SetSingleLineReplies(singleLineReplies);
  }

  bool SingleLineReplies() const
  {
    return client.Control().SingleLineReplies();
  }

  void Reply(ftp::ReplyCode code, const std::string& message)
  {
    UnlockGuard unlock(*this); (void) unlock;
    client.Control().Reply(code, message);
  }

  void PartReply(ftp::ReplyCode code, const std::string& message)
  {
    UnlockGuard unlock(*this); (void) unlock;
    client.Control().PartReply(code, message);
  }
};

} /* script namespace */

#endif
