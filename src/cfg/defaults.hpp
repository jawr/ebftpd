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
extern const long long         defaultEmptyNuke;
extern const int               defaultMaxSitecmdLines;
extern const WeekStart         defaultWeekStart;
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
extern const Database          defaultDatabase;
extern const PathFilter        defaultPathFilter;
extern const NukedirStyle      defaultNukedirStyle;
extern const IdleTimeout       defaultIdleTimeout;
extern const AllowFxp          defaultAllowFxp;
extern const MaxUsers          defaultMaxUsers;
extern const Lslong            defaultLslong;
extern const SimXfers          defaultSimXfers;

} /* cfg namespace */

#endif
