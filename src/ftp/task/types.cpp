#include "ftp/task/types.hpp"
#include "stats/util.hpp"

namespace ftp { namespace task
{

std::string WhoUser::Action() const
{
  boost::posix_time::ptime now(boost::posix_time::second_clock::local_time());
  std::ostringstream action;
  if (tState.Type() != ftp::TransferType::Upload && 
      tState.Type() != ftp::TransferType::Download)
  {
    action << command;
    if (action.str().empty()) action << "IDLE " << idleTime;
  }
  else
  {
    if (tState.Type() == ftp::TransferType::Upload)
      action << "UP @ ";
    else
      action << "DN @ ";
    action << stats::AutoUnitSpeedString(stats::
        CalculateSpeed(tState.Bytes(), tState.StartTime(), now));
  }
  
  return action.str();
}

} /* task namespace */
} /* ftp namespace */
