//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

long long UploadSpeedLimit(const cfg::SpeedLimit& limit)
{
  return limit.Uploads();
}

long long DownloadSpeedLimit(const cfg::SpeedLimit& limit)
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
