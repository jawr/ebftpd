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

#ifndef __CFG_DEFAULTS_HPP
#define __CFG_DEFAULTS_HPP

#include "cfg/config.hpp"
#include "fs/mode.hpp"

namespace cfg
{

extern const long long         defaultFreeSpace;
extern const char*             defaultSitenameLong;
extern const char*             defaultSitenameShort;
extern const bool              defaultBouncerOnly;
extern const Log               defaultSecurityLog;
extern const Log               defaultDatabaseLog;
extern const Log               defaultEventLog;
extern const Log               defaultErrorLog;
extern const Log               defaultDebugLog;
extern const Log               defaultSiteopLog;
extern const TransferLog       defaultTransferLog;
extern const bool              defaultDlIncomplete;
extern const int               defaultTotalUsers;
extern const int               defaultMultiplierMax;
extern const int               defaultMaxSitecmdLines;
extern const EPSVFxp           defaultEpsvFxp;
extern const int               defaultMaximumRatio;
extern const int               defaultDirSizeDepth;
extern const bool              defaultAsyncCRC;
extern const bool              defaultIdentLookup;
extern const bool              defaultDnsLookup;
extern const LogAddresses      defaultLogAddresses;
extern const mode_t            defaultUmask;
extern const int               defaultLogLines;
extern const size_t            defaultDataBufferSize;
extern const char*             defaultTlsControl;
extern const char*             defaultTlsListing;
extern const char*             defaultTlsData;
extern const char*             defaultTlsFxp;
extern const char*             defaultDatabaseName;
extern const char*             defaultDatabaseAddress;
extern const int               defaultDatabasePort;
extern const PathFilter        defaultPathFilter;
extern const NukeStyle         defaultNukeStyle;
extern const IdleTimeout       defaultIdleTimeout;
extern const AllowFxp          defaultAllowFxp;
extern const MaxUsers          defaultMaxUsers;
extern const Lslong            defaultLslong;
extern const SimXfers          defaultSimXfers;
extern const NukeMax           defaultNukeMax;

} /* cfg namespace */

#endif
