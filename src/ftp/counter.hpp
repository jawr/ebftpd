#ifndef __FTP_COUNTER_HPP
#define __FTP_COUNTER_HPP

#include "transfercounter.hpp"
#include "logincounter.hpp"
#include "speedcounter.hpp"

namespace ftp
{

enum class CounterResult
{
  PersonalFail,
  GlobalFail,
  Okay
};

class Counter
{
  static LoginCounter logins;
  static TransferCounter uploads;
  static TransferCounter downloads;
  static SpeedCounter uploadSpeeds;
  static SpeedCounter downloadSpeeds;
  
public:
  static LoginCounter& Login() { return logins; }
  static TransferCounter& Upload() { return uploads; }
  static TransferCounter& Download() { return downloads; }
  static SpeedCounter& UploadSpeeds() { return uploadSpeeds; }
  static SpeedCounter& DownloadSpeeds() { return downloadSpeeds; }
};

} /* ftp namespace */

#endif
