#ifndef __ACL_FLAGS_HPP
#define __ACL_FLAGS_HPP

#include <string>

namespace acl
{

enum class Flag : char
{
  Siteop = '1',
  Gadmin = '2',
  Template = '3',
  Exempt = '4',
  Color = '5',
  Deleted = '6',
  Useredit = '7',
  Anonymous = '8',
  Nuke = 'A',
  Unnuke = 'B',
  Undupe = 'C',
  Kick = 'D',
  Kill = 'E',
  Take = 'F',
  Give = 'G',
  Users = 'H',
  Idler = 'I',
  Custom1 = 'J',
  Custom2 = 'K',
  Custom3 = 'L',
  Custom4 = 'M',
  Custom5 = 'N'
};

bool ValidFlags(const std::string& flags);

} /* acl namespace */

#ifndef __ACL_FLAGS_CPP
extern const std::string validFlags;
#endif

#endif
