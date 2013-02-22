#include "cmd/site/reload.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"

namespace cmd { namespace site
{

void RELOADCommand::Execute()
{
  typedef ftp::task::ReloadConfig::Result Result;
  std::future<std::pair<Result, Result>> future;
  std::make_shared<ftp::task::ReloadConfig>(future)->Push();
  
  future.wait();
  
  std::stringstream os;
  
  const auto result = future.get();
  auto& configResult = result.first;
  auto& templatesResult = result.second;

  bool furtherDetails = false;
  if (configResult == Result::Fail)
  {
    os << "Config failed to reload.";
    furtherDetails = true;
  }
  else
    os << "Config reloaded.";
  
  if (configResult == Result::StopStart)
  {
    os << "\nSome of the options changed require a full stop start.";
    furtherDetails = true;
  }
    
  if (templatesResult == Result::Fail)
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
