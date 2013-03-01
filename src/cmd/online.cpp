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

std::string CompileWhosOnline(const std::string& id, text::Template& templ)
{
  std::vector<ftp::OnlineClient> clients;
  
  {
    ftp::OnlineReader reader(id);  
    
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
  
  int count = 0;
  int idlers = 0;
  int active = 0;
  int uploaders = 0;
  int downloaders = 0;
  double upFastest = 0;
  double downFastest = 0;
  double upSlowest = -1;
  double downSlowest = -1;
  double upTotalSpeed = 0;
  double downTotalSpeed = 0;
  
  for (const auto& client : clients)
  {
    auto it = std::find_if(users.begin(), users.end(), 
                    [&](const acl::User& user)
                    {
                      return user.ID() == client.uid;
                    });
    if (it == users.end()) continue;
    
    body.RegisterValue("count", ++count);
    body.RegisterValue("user", it->Name());
    body.RegisterValue("group", it->PrimaryGroup());
    body.RegisterValue("tagline", it->Tagline());
    
    body.RegisterValue("ident", client.ident);
    body.RegisterValue("ip", client.ip);
    body.RegisterValue("hostname", client.hostname);
    body.RegisterValue("work_dir", client.workDir);
    body.RegisterValue("ident_address", client.ident + std::string("@") + client.hostname);
    
    // below needs santising for hideinwho config option
    
    std::ostringstream action;
    if (client.xfer) // transfering
    {
      double speed = stats::CalculateSpeed(client.xfer->bytes, client.xfer->start, 
                        boost::posix_time::microsec_clock::local_time()) / 1024;
      
      if (client.xfer->direction == stats::Direction::Upload)
      {
        ++uploaders;
        upTotalSpeed += speed;
        if (upSlowest == -1) upSlowest = speed;
        else upSlowest = std::min(upSlowest, speed);
        upFastest = std::max(upFastest, speed);
        action << "UP @ ";
      }
      else
      {
        ++downloaders;
        downTotalSpeed += speed;
        if (downSlowest == -1) downSlowest = speed;
        else downSlowest = std::max(downSlowest, speed);
        downFastest = std::max(downFastest, speed);
        action << "DN @ ";
      }
      
      action << stats::AutoUnitSpeedString(speed);
    }
    else // not transfering
    {
      if (client.IsIdle()) // idle
      {
        ++idlers;
        action << "IDLE " << (boost::posix_time::second_clock::local_time() - client.lastCommand);
      }
      else // executing command
      {
        ++active;
        action << client.command;
      }
    }
    
    body.RegisterValue("action", action.str());
    os << body.Compile();
  }
  
  if (upSlowest == -1) upSlowest = 0;
  if (downSlowest == -1) downSlowest = 0;
  
  auto& foot = templ.Foot();
  foot.RegisterValue("online_users", count);
  foot.RegisterValue("all_online_users", count);
  foot.RegisterValue("idlers", idlers);
  foot.RegisterValue("active", active);
  foot.RegisterValue("uploaders", uploaders);
  foot.RegisterValue("downloaders", downloaders);
  foot.RegisterValue("up_fastest_speed", upFastest);
  foot.RegisterValue("up_slowest_speed", upSlowest);
  foot.RegisterValue("down_fastest_speed", downFastest);
  foot.RegisterValue("down_slowest_speed", downSlowest);
  foot.RegisterValue("up_total_speed", upTotalSpeed);
  foot.RegisterValue("up_avg_speed", uploaders == 0 ? 0 : upTotalSpeed / uploaders);
  foot.RegisterValue("down_total_speed", downTotalSpeed);
  foot.RegisterValue("down_avg_speed", downloaders == 0 ? 0 : downTotalSpeed / downloaders);
  foot.RegisterValue("max_online_users", cfg::Config::MaxOnline().Users());
  os << foot.Compile();
  return os.str();
}

std::string CompileWhosOnline(text::Template& templ)
{
  return CompileWhosOnline(ftp::SharedMemoryID(), templ);
}

} /* cmd namespace */
