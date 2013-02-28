#include <algorithm>
#include <sstream>
#include "cmd/online.hpp"
#include "ftp/online.hpp"
#include "acl/user.hpp"
#include "text/template.hpp"
#include "stats/util.hpp"
#include "cfg/get.hpp"

namespace cmd
{

std::string CompileWhosOnline(text::Template& templ)
{
  std::vector<ftp::OnlineClient> clients;
  
  {
    ftp::OnlineReader reader(ftp::SharedMemoryID());  
    
    {
      ftp::OnlineReaderLock lock(reader);
      std::copy(reader.begin(), reader.end(), std::back_inserter(clients));
    }
  }
  
  std::ostringstream multiStr;
  for (const auto& client : clients)
    multiStr << "-" << acl::UIDToName(client.uid) << " ";
    
  auto users = acl::User::GetUsers(multiStr.str());

  std::ostringstream os;
  auto& head = templ.Head();
  os << head.Compile();
  
  auto& body = templ.Body();
  
  int index = 0;
  for (const auto& client : clients)
  {
    auto it = std::find_if(users.begin(), users.end(), 
                    [&](const acl::User& user)
                    {
                      return user.ID() == client.uid;
                    });
    if (it == users.end()) continue;
    
    body.RegisterValue("index", ++index);
    body.RegisterValue("user", it->Name());
    body.RegisterValue("group", it->PrimaryGroup());
    body.RegisterValue("tagline", it->Tagline());
    
    body.RegisterValue("ident", client.ident);
    body.RegisterValue("ip", client.ip);
    body.RegisterValue("hostname", client.hostname);
    body.RegisterValue("work_dir", client.workDir);
    body.RegisterValue("ident_address", client.ident + "@" + client.hostname);
    
    // below needs santising for hideinwho config option
    
    std::ostringstream action;
    if (client.xfer) // transfering
    {
      if (client.xfer->direction == stats::Direction::Upload)
        action << "UP @ ";
      else
        action << "DN @ ";
      
      action << stats::AutoUnitSpeedString(stats::CalculateSpeed(client.xfer->bytes, 
                    client.xfer->start, boost::posix_time::microsec_clock::local_time()) / 1024);
    }
    else // not transfering
    {
      if (client.command.empty()) // idle
      {
        action << "IDLE " << (boost::posix_time::second_clock::local_time() - client.lastCommand);
      }
      else // executing command
      {
        action << client.command;
      }
    }
    
    body.RegisterValue("action", action.str());
    os << body.Compile();
  }
  
  auto& foot = templ.Foot();
  foot.RegisterValue("online_users", index);
  foot.RegisterValue("all_online_users", index);
  foot.RegisterValue("max_online_users", cfg::Get().MaxUsers().Users());
  os << foot.Compile();
  return os.str();
}

} /* cmd namespace */
