#include <sstream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "exec/util.hpp"
#include "ftp/client.hpp"
#include "acl/group.hpp"

namespace exec
{

std::vector<std::string> BuildEnv(ftp::Client& client)
{
  std::ostringstream os;
  os << "USER=" << client.User().Name() << "\n"
     << "FLAGS=" << client.User().Flags() << "\n"
     << "TAGLINE=" << client.User().Tagline() << "\n"
     << "GROUP=" << acl::GIDToName(client.User().PrimaryGID()) << "\n"
     << "RATIO=" << client.User().DefaultRatio() << "\n"
     << "HOST=" << client.Ident() << "@" << client.IP();
  
  std::string envStr(os.str());
  std::vector<std::string> env;
  boost::split(env, envStr, boost::is_any_of("\n"), boost::token_compress_on);
  return env;
}

} /* exec namespace */
