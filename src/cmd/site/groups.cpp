#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "cmd/site/groups.hpp"
#include "acl/group.hpp"
#include "acl/user.hpp"
#include "db/user/user.hpp"
#include "db/group/group.hpp"

namespace cmd { namespace site
{

void GROUPSCommand::Execute()
{
  boost::ptr_vector<acl::Group> groups; 

  std::ostringstream os;

  db::group::GetAll(groups);

  if (groups.size() > 0)
  {
    os << "(Users)  Name      Group Description";
    os << "\n----------------------------------------------------------------------";

    boost::ptr_vector<acl::User> users; 
    for (auto& group: groups)
    {
      db::user::UsersByACL(users, "=" + group.Name());
      os << "\n(" << users.size() << ") " << group.Name();
    }
  }
  else
    os << "No groups added.";

  control.Reply(ftp::CommandOkay, os.str());
}

// end
}
}
