#ifndef __FTP_ENUMS_HPP
#define __FTP_ENUMS_HPP

namespace ftp
{

enum class ClientState
{
  LoggedOut,
  WaitingPassword,
  LoggedIn,
  Finished,
  NotBeforeAuth,
  AnyState
};

enum class LogAddresses
{
  NotLogging,
  Error,
  Normal
};

}

#endif
