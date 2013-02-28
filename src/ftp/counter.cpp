#include <sstream>
#include "ftp/counter.hpp"
#include "cfg/get.hpp"

namespace ftp
{
namespace
{
int MaximumUploads()
{
  return cfg::Get().SimXfers().MaxUploads();
}

int MaximumDownloads()
{
  return cfg::Get().SimXfers().MaxDownloads();
}

int UploadSpeedLimit(const cfg::SpeedLimit& limit)
{
  return limit.Uploads();
}

int DownloadSpeedLimit(const cfg::SpeedLimit& limit)
{
  return limit.Downloads();
}

}

LoginCounter Counter::logins;
TransferCounter Counter::uploads(MaximumUploads);
TransferCounter Counter::downloads(MaximumDownloads);
SpeedCounter Counter::uploadSpeeds(UploadSpeedLimit);
SpeedCounter Counter::downloadSpeeds(DownloadSpeedLimit);

} /* ftp namespace */
