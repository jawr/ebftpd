#include "cmd/site/reload.hpp"
#include "ftp/listener.hpp"
#include "ftp/task/task.hpp"
#include "cfg/config.hpp"

namespace cmd { namespace site
{

void RELOADCommand::Execute()
{
  using namespace ftp::task;

  boost::unique_future<ReloadConfig::Result> future;
  ftp::TaskPtr task(new ReloadConfig(future));
  ftp::Listener::PushTask(task);
  future.wait();
  
  std::stringstream os;
  
  ReloadConfig::Result result = future.get();
  if (result == ReloadConfig::Result::Fail)
  {
    os << "Config failed to reload.\n";
    os << "See SITE LOGS ERROR for further details.";
  }

  os << "Config reloaded.";

  if (result == ReloadConfig::Result::StopStart)
  {
    os << "\nSome of the options changed require a full stop start.\n";
    os << "See SITE LOGS ERROR for details.";
  }
  
  control.Reply(ftp::CommandOkay, os.str());
}

} /* site namespace */
} /* cmd namespace */
