#include <sstream>
#include "util/string.hpp"
#include "exec/util.hpp"
#include "ftp/client.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"

namespace exec
{

std::vector<std::string> BuildEnv(ftp::Client& client)
{
  std::ostringstream os;
  os << "USER=" << client.User().Name() << "\n"
     << "UID=" << client.User().ID() << "\n"
     << "FLAGS=" << client.User().Flags() << "\n"
     << "TAGLINE=" << client.User().Tagline() << "\n"
     << "GROUP=" << acl::GIDToName(client.User().PrimaryGID()) << "\n"
     << "GID=" << client.User().PrimaryGID() << "\n"
     << "HOST=" << client.Ident() << "@" << client.IP();
  
  std::string envStr(os.str());
  std::vector<std::string> env;
  util::Split(env, envStr, "\n", true);
  return env;
}

} /* exec namespace */
