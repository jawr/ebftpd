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
