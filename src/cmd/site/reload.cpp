#include "cmd/site/reload.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"

namespace cmd { namespace site
{

void RELOADCommand::Execute()
{
  using namespace ftp::task;

  boost::unique_future<std::pair<ReloadConfig::Result, ReloadConfig::Result>> future;
  ftp::TaskPtr task(new ReloadConfig(future));
  ftp::Listener::PushTask(task);
  future.wait();
  
  std::stringstream os;
  
  const auto result = future.get();
  auto& configResult = result.first;
  auto& templatesResult = result.second;

  bool furtherDetails = false;
  if (configResult == ReloadConfig::Result::Fail)
  {
    os << "Config failed to reload.";
    furtherDetails = true;
  }
  else
    os << "Config reloaded.";
  
  if (configResult == ReloadConfig::Result::StopStart)
  {
    os << "\nSome of the options changed require a full stop start.";
    furtherDetails = true;
  }
    
  if (templatesResult == ReloadConfig::Result::Fail)
  {
    os << "\nTemplates failed to reload.";
    furtherDetails = true;
  }
  else
    os << "\nTemplates reloaded.";

  if (furtherDetails)
  {
    os << "\nSee SITE LOGS ERROR for further details.";    
  }
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
