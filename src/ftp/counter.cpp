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

int UploadSpeedLimit(const cfg::setting::SpeedLimit& limit)
{
  return limit.UlLimit();
}

int DownloadSpeedLimit(const cfg::setting::SpeedLimit& limit)
{
  return limit.DlLimit();
}

}

LoginCounter Counter::logins;
TransferCounter Counter::uploads(MaximumUploads);
TransferCounter Counter::downloads(MaximumDownloads);
SpeedCounter Counter::uploadSpeeds(UploadSpeedLimit);
SpeedCounter Counter::downloadSpeeds(DownloadSpeedLimit);

} /* ftp namespace */
