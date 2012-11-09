#include <algorithm>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "cmd/site/msg.hpp"
#include "logs/logs.hpp"
#include "db/user/user.hpp"
#include "db/mail/mail.hpp"
#include "db/mail/message.hpp"

namespace cmd { namespace site
{

cmd::Result MSGCommand::Read(bool trash)
{
  if (args.size() != 2) return cmd::Result::SyntaxError;
  control.Reply(ftp::NotImplemented, "Not implemented.");
  return cmd::Result::Okay;
}

cmd::Result MSGCommand::Send()
{
  if (args.size() < 4) return cmd::Result::SyntaxError;

  boost::trim(args[2]);
  std::vector<std::string> recipients;
  if (args[2] != "*")
  {
    boost::split(recipients, args[2], boost::is_any_of(" "), boost::token_compress_on);
    for (auto& recipient : recipients)
      if (recipient[0] != '=' && recipient[0] != '-')
        recipient.insert(recipient.begin(), '-');
  }
  
  std::string body(boost::join(std::vector<std::
      string>(args.begin() + 3, args.end()), " "));
  
  boost::ptr_vector<acl::User> users;
  if (recipients.empty()) db::user::GetAll(users);
  else
  {
    for (auto& recipient : recipients)
      db::user::UsersByACL(users, recipient);
      
    std::sort(users.begin(), users.end(), 
      [](const acl::User& u1, const acl::User& u2)
      { return u1.UID() < u2.UID(); });
      
    users.erase(std::unique(users.begin(), users.end(), 
      [](const acl::User& u1, const acl::User& u2)
      { return u1.UID() == u2.UID(); }), users.end());
  }
    
  boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  unsigned sentCount = 0;
  for (auto& user : users)
  {
    if (user.Deleted()) continue;
    if (user.UID() == client.User().UID()) continue;
    db::mail::Send(db::mail::Message(client.User().Name(), user.UID(), body, now));
    ++sentCount;
  }
  
  std::ostringstream os;
  os << "Message sent to " << sentCount << " user(s).";
  control.Reply(ftp::CommandOkay, os.str());
  return cmd::Result::Okay;
}

cmd::Result MSGCommand::Save()
{
  if (args.size() != 2) return cmd::Result::SyntaxError;
  control.Reply(ftp::NotImplemented, "Not implemented.");
  return cmd::Result::Okay;
}

cmd::Result MSGCommand::Purge()
{
  if (args.size() != 2) return cmd::Result::SyntaxError;
  control.Reply(ftp::NotImplemented, "Not implemented.");
  return cmd::Result::Okay;
}

cmd::Result MSGCommand::List()
{
  if (args.size() != 2) return cmd::Result::SyntaxError;
  std::vector<db::mail::Message> mail(db::mail::Get(client.User().UID()));
  if (mail.empty())
  {
    control.Reply(ftp::CommandOkay, "Your mail box is empty.");
    return cmd::Result::Okay;
  }
  
  control.PartReply(ftp::CommandOkay, ".----.------------.-------.----------------------.----------------------.");
  control.PartReply(ftp::CommandOkay, "| ## | Sender     | Trash | Date/Time            | Message Start        |");
  control.PartReply(ftp::CommandOkay, "|----+------------+-------+----------------------+----------------------|");
  
  unsigned index = 1;
  for (auto& message : mail)
  {
    std::ostringstream os;
    os << "| " << std::setfill('0') << std::setw(2) << index << " | " << std::setfill(' ')
       << std::left << std::setw(10) << message.Sender().substr(0, 10) << " | "
       << std::left << std::setw(5) << (message.Trash() ? " Yes" : " No") << " | "
       << std::left << message.TimeSent() << " | ";
    if (message.Body().length() > 20)
      os << std::setw(17) << message.Body().substr(0, 17) << " .. |";
    else
      os << std::setw(20) << message.Body() << " |";
    control.PartReply(ftp::CommandOkay, os.str());
  }
  control.PartReply(ftp::CommandOkay, "`----'------------'-------'----------------------'----------------------'");
  control.Reply(ftp::CommandOkay, "End of mail box list.");
  return cmd::Result::Okay;
}

cmd::Result MSGCommand::Execute()
{
  std::string cmd(boost::to_lower_copy(args[1]));
  if (cmd == "read") return Read();
  else if (cmd == "send") return Send();
  else if (cmd == "save") return Save();
  else if (cmd == "purge") return Purge();
  else if (cmd == "list") return List();
  return cmd::Result::SyntaxError;
}

} /* site namespace */
} /* cmd namespace */
