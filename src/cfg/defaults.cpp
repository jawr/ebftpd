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

#include "defaults.hpp"

namespace cfg
{

const long long         defaultFreeSpace          = 1048576;  // 1024MB
const char*             defaultSitenameLong       = "EBFTPD";
const char*             defaultSitenameShort      = "EB";
const bool              defaultBouncerOnly        = false;
const Log               defaultSecurityLog        ("security",  true,   true,   0);
const Log               defaultDatabaseLog        ("database",  true,   true,   0);
const Log               defaultEventLog           ("events",    true,   true,   0);
const Log               defaultErrorLog           ("errors",    true,   true,   0);
const Log               defaultDebugLog           ("debug",     false,  false,  0);
const Log               defaultSiteopLog          ("siteop",    true,   true,   0);
const TransferLog       defaultTransferLog        ("transfer",  false,  false,  0,  false,  false);
const bool              defaultDlIncomplete       = true;
const int               defaultTotalUsers         = -1;       // unlimited
const int               defaultMultiplierMax      = 10;
const long long         defaultEmptyNuke          = 102400;   // 100MB
const int               defaultMaxSitecmdLines    = 100;
const WeekStart         defaultWeekStart          = WeekStart::Sunday;
const EPSVFxp           defaultEpsvFxp            = EPSVFxp::Allow;
const int               defaultMaximumRatio       = 10;
const int               defaultDirSizeDepth       = 2;
const bool              defaultAsyncCRC           = false;
const bool              defaultIdentLookup        = true;
const bool              defaultDnsLookup          = true;
const LogAddresses      defaultLogAddresses       = LogAddresses::Always;
const mode_t            defaultUmask              = fs::CurrentUmask();
const int               defaultLogLines           = 100;
const size_t            defaultDataBufferSize     = 16384;
const char*             defaultTlsControl         = "*";    // enforced
const char*             defaultTlsListing         = "*";    // enforced
const char*             defaultTlsData            = "!*";   // not enforced
const char*             defaultTlsFxp             = "!*";   // not enforced
const Database          defaultDatabase           ("ebftpd", "localhost", 27017, "", "");
const PathFilter        defaultPathFilter         ("^[[\\]A-Za-z0-9_'()[:space:]][[\\]A-Za-z0-9_.'()[:space:]-]+$", "*");
const NukedirStyle      defaultNukedirStyle       ("NUKED-%N", NukedirStyle::Keep, 1024);
const IdleTimeout       defaultIdleTimeout        (7200 /* max */, 1 /* min */, 900 /* default */);
const AllowFxp          defaultAllowFxp           (true /* dn */, true /* up */, false /* log */ , "*" /* acl */);
const MaxUsers          defaultMaxUsers           (50 /* users */, 5 /* exempt */);
const Lslong            defaultLslong             ("l", 2);
const SimXfers          defaultSimXfers           (-1 /* down */, -1 /* down */); // unlimited
const NukeMax           defaultNukeMax            (10 /* mutiplier */, 50 /* percent */);

} /* cfg namespace */
