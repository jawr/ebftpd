#ifndef __CMD_SITE_GROUP_HPP
#define __CMD_SITE_GROUP_HPP

#include "cmd/command.hpp"

namespace acl
{
class GroupProfile;
class Group;
}

namespace text
{
class TemplateSection;
}

namespace cmd { namespace site
{

class GROUPCommand : public Command
{
  void PopulateHeadOrFoot(const acl::Group& group, 
        const acl::GroupProfile& profile, text::TemplateSection& tmpl);

public:
  GROUPCommand(ftp::Client& client, const std::string& argStr, const Args& args) :
    Command(client, client.Control(), client.Data(), argStr, args) { }

  void Execute();
};

} /* site namespace */
} /* cmd namespace */

#endif
