#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "exec/util.hpp"
#include "ftp/client.hpp"
#include "acl/groupcache.hpp"

namespace exec
{

std::vector<std::string> BuildEnv(ftp::Client& client)
{
  std::ostringstream os;
  os << "USER=" << client.User().Name() << "\n"
     << "FLAGS=" << client.User().Flags() << "\n"
     << "TAGLINE=" << client.UserProfile().Tagline() << "\n"
     << "GROUP=" << acl::GroupCache::GIDToName(client.User().PrimaryGID()) << "\n"
     << "RATIO=" << client.UserProfile().Ratio() << "\n"
     << "HOST=" << client.Ident() << "@" << client.Address();
  
  std::string envStr(os.str());
  std::vector<std::string> env;
  boost::split(env, envStr, boost::is_any_of("\n"), boost::token_compress_on);
  return env;
}

} /* exec namespace */
