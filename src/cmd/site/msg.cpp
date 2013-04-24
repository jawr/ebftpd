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

#include <cctype>
#include <algorithm>
#include <map>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/msg.hpp"
#include "logs/logs.hpp"
#include "db/mail/mail.hpp"
#include "db/mail/message.hpp"
#include "util/string.hpp"
#include "util/enumstrings.hpp"
#include "cmd/error.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "acl/misc.hpp"
#include "acl/flags.hpp"
#include "acl/user.hpp"

namespace cmd { namespace site
{

void MSGCommand::Read(const std::vector<db::mail::Message>& mail)
{
  control.PartReply(ftp::CommandOkay, ".----.--------------------.----------------------------------------------.");
  
  unsigned index = 0;
  for (auto& message : mail)
  {
    if (index > 0)
      control.PartReply(ftp::CommandOkay, "+----.--------------------.----------------------------------------------+");
    std::ostringstream when;
    when << message.TimeSent();
    std::ostringstream os;
    os << "| " << std::right << std::setw(2) << std::setfill('0') << ++index << std::setfill(' ')
       << " | Sender: " << std::left << std::setw(10) << message.Sender().substr(0, 10)
       << " | When: " << std::setw(38) << when.str().substr(0, 38) << " | ";
    control.PartReply(ftp::CommandOkay, os.str());
    control.PartReply(ftp::CommandOkay, "+----`--------------------`----------------------------------------------+");
    
    std::string body = message.Body();
    while (!body.empty())
    {
      os.str("");
      os << "| " << std::setw(70) << std::left << util::WordWrap(body, 70) << " |";
      control.PartReply(ftp::CommandOkay, os.str());
    }
    
    if (message.Status() == db::mail::Status::Unread) db::mail::Trash(message);
  }
  
  control.PartReply(ftp::CommandOkay, "`------------------------------------------------------------------------'");
}

void MSGCommand::Read()
{
  int index = -1;
  if (args.size() > 3) throw cmd::SyntaxError();
  if (args.size() == 3)
  {
    try
    {
      index = util::StrToInt(args[2]);
      if (index < 1)
      {
        control.Reply(ftp::ActionNotOkay, "Index must be 1 or larger.");
        return;
      }
    }
    catch (const std::bad_cast&)
    { throw cmd::SyntaxError(); }
  }
  
  std::vector<db::mail::Message> mail(db::mail::Get(client.User().ID()));
  if (mail.empty())
  {
    control.Reply(ftp::CommandOkay, "Your mail box is empty.");
    return;
  }
  
  if (index != -1)
  {
    if (mail.size() < static_cast<unsigned>(index))
    {
      control.Reply(ftp::ActionNotOkay, "No message with index " + args[2] + ".");
      return;
    }
    
    Read({ mail[index - 1] });
    control.Reply(ftp::CommandOkay, "End of message.");
  }
  else
  {
    mail.erase(std::remove_if(mail.begin(), mail.end(), [](const db::mail::Message& m)
      { return m.Status() != db::mail::Status::Unread; }), mail.end());
    if (mail.empty())
      control.Reply(ftp::CommandOkay, "No unread mail to read.");
    else
    {
      Read(mail);
      control.Reply(ftp::CommandOkay, "Messages moved to the trash.\n"
              "Trash will be purged at disconnect if not saved.");
    }
  }
}

void MSGCommand::Send()
{
  if (args.size() < 4) throw cmd::SyntaxError();

  util::Trim(args[2]);
  std::vector<std::string> recipients;
  if (args[2] != "*")
  {
    util::Trim(args[2]);
    util::Split(recipients, args[2], " ", true);
    if (recipients.size() > 1 &&
        !acl::AllowSiteCmd(client.User(), "msg{")) 
    {
      throw cmd::PermissionError();
    }
    
    bool equalsOp = false;
    for (auto& recipient : recipients)
    {
      if (recipient[0] == '=')
        equalsOp = true;
      else if (recipient[0] != '-')
        recipient.insert(recipient.begin(), '-');
    }
    
    if (equalsOp && !acl::AllowSiteCmd(client.User(), "msg=")) 
    throw cmd::PermissionError();
  }
  
  std::string body(util::Join(std::vector<std::
      string>(args.begin() + 3, args.end()), " "));
  
  std::vector<acl::User> users;
  if (recipients.empty()) 
  {
    if (!acl::AllowSiteCmd(client.User(), "msg*")) throw cmd::PermissionError();
    users = acl::User::GetUsers();
  }
  else
  {
    for (auto& recipient : recipients)
    {
      auto tempUsers = acl::User::GetUsers(recipient);
      users.insert(users.end(), tempUsers.begin(), tempUsers.end());
    }
      
    std::sort(users.begin(), users.end(), 
      [](const acl::User& u1, const acl::User& u2)
      { return u1.ID() < u2.ID(); });
      
    users.erase(std::unique(users.begin(), users.end(), 
      [](const acl::User& u1, const acl::User& u2)
      { return u1.ID() == u2.ID(); }), users.end());
  }
    
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  unsigned sentCount = 0;
  for (auto& user : users)
  {
    if (user.HasFlag(acl::Flag::Deleted)) continue;
    if (user.ID() == client.User().ID()) continue;
    db::mail::Send(db::mail::Message(client.User().Name(), user.ID(), body, now));
    ++sentCount;
  }
  
  std::ostringstream os;
  os << "Message sent to " << sentCount << " user(s).";
  control.Reply(ftp::CommandOkay, os.str());
}

void MSGCommand::SaveTrash()
{
  unsigned saved = db::mail::SaveTrash(client.User().ID());
  std::ostringstream os;
  os << saved << " message(s) saved from your trash.";
  control.Reply(ftp::CommandOkay, os.str());
}

void MSGCommand::Save()
{
  if (args.size() == 2) return SaveTrash();
  else if (args.size() > 3) throw cmd::SyntaxError();

  int index;
  try
  {
    index = util::StrToInt(args[2]);
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  bool purged = db::mail::Save(client.User().ID(), index - 1);
  if (!purged) control.Reply(ftp::ActionNotOkay, "Failed to saved message with index " + args[2] + ".");
  else control.Reply(ftp::CommandOkay, "Message " + args[2] + " saved.");
}

void MSGCommand::PurgeTrash()
{
  unsigned purged = db::mail::PurgeTrash(client.User().ID());
  std::ostringstream os;
  os << purged << " message(s) purged from your trash.";
  control.Reply(ftp::CommandOkay, os.str());
}

void MSGCommand::Purge()
{
  if (args.size() == 2) return PurgeTrash();
  else if (args.size() > 3) throw cmd::SyntaxError();

  int index;
  try
  {
    index = util::StrToInt(args[2]);
  }
  catch (const std::bad_cast&)
  {
    throw cmd::SyntaxError();
  }
  
  bool purged = db::mail::Purge(client.User().ID(), index - 1);
  if (!purged) control.Reply(ftp::ActionNotOkay, "Failed to purge message with index " + args[2] + ".");
  else control.Reply(ftp::CommandOkay, "Message " + args[2] + " purged.");
}

void MSGCommand::List()
{
  if (args.size() != 2) throw cmd::SyntaxError();
  
  std::vector<db::mail::Message> mail(db::mail::Get(client.User().ID()));
  if (mail.empty())
  {
    control.Reply(ftp::CommandOkay, "Your mail box is empty.");
    return;
  }

  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("msg.list"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  text::TemplateSection& head = templ->Head();
  text::TemplateSection& body = templ->Body();
  text::TemplateSection& foot = templ->Foot();
  
  std::ostringstream os;
  os << head.Compile();
  
  std::map<db::mail::Status, unsigned> totals;
  
  unsigned index = 0;
  for (auto& message : mail)
  {
    body.RegisterValue("index", ++index);
    body.RegisterValue("sender", message.Sender());
    body.RegisterValue("status", util::ToUpperCopy(util::EnumToString(message.Status())));
    body.RegisterValue("when", boost::posix_time::to_simple_string(message.TimeSent()));
    body.RegisterValue("body", message.Body());
    
    os << body.Compile();
    
    ++totals[message.Status()];
  }
  
  foot.RegisterValue("total", mail.size());
  foot.RegisterValue("total_unread", totals[db::mail::Status::Unread]);
  foot.RegisterValue("total_trash", totals[db::mail::Status::Trash]);
  foot.RegisterValue("total_saved", totals[db::mail::Status::Saved]);
  
  os << foot.Compile();

  control.Reply(ftp::CommandOkay, os.str());
}

void MSGCommand::Execute()
{
  std::string cmd(util::ToLowerCopy(args[1]));
  if (cmd == "read") return Read();
  else if (cmd == "send") return Send();
  else if (cmd == "save") return Save();
  else if (cmd == "purge") return Purge();
  else if (cmd == "list") return List();
  throw cmd::SyntaxError();
}

} /* site namespace */
} /* cmd namespace */
