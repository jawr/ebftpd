#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/groups.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "db/user/user.hpp"
#include "db/group/group.hpp"

namespace cmd { namespace site
{

cmd::Result GROUPSCommand::Execute()
{
  boost::ptr_vector<acl::Group> groups = db::group::GetAllPtr();

  std::ostringstream os;

  if (groups.size() > 0)
  {
    os << "(Users)  Name      Group Description";
    os << "\n----------------------------------------------------------------------";

    for (auto& group: groups)
    {
      std::vector<acl::User> users = db::user::GetByACL("=" + group.Name());
      os << "\n(" << users.size() << ") " << group.Name();
    }
  }
  else
    os << "No groups added.";

  control.MultiReply(ftp::CommandOkay, os.str());
  
  return cmd::Result::Okay;
}

// end
}
}
