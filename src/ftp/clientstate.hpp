#ifndef __FTP_CLIENTSTATE_HPP
#define __FTP_CLIENTSTATE_HPP

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

} /* ftp namespace */

#endif
