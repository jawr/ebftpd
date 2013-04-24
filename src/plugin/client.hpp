//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef __PLUGIN_CLIENT_HPP
#define __PLUGIN_CLIENT_HPP

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional.hpp>
#include "ftp/client.hpp"
#include "ftp/control.hpp"
#include "acl/user.hpp"
#include "plugin/user.hpp"
#include "plugin/error.hpp"
#include "plugin/plugin.hpp"

namespace plugin
{

class Client
{
  ftp::Client* client;
  mutable boost::optional<plugin::User> user;
  
public:
  Client() { throw NotConstructable(); }
  explicit Client(ftp::Client& client) :
    client(&client)
  {
    if (client.State() == ftp::ClientState::LoggedIn)
    {
      user.reset(plugin::User(client.User().ShallowCopy()));
    }
  }
  
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
    if (!user)
    {
      if (client->State() != ftp::ClientState::LoggedIn) return boost::none;
      user.reset(plugin::User(client->User().ShallowCopy()));
    }
    return boost::optional<plugin::User&>(*user);
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
    std::cout << long(this) << " " << long(&(*user)) << std::endl;
    UnlockGuard unlock; (void) unlock;
    client->Control().PartReply(code, message);
  }
};

} /* script namespace */

#endif
