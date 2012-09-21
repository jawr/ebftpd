#ifndef __ACL_FLAGS_HPP
#define __ACL_FLAGS_HPP

#include <string>

namespace acl
{

enum Flag
{
	FlagSiteop = '1',
  FlagGadmin = '2',
  FlagGlock = '3',
  FlagExempt = '4',
  FlagColor = '5',
  FlagDeleted = '6',
  FlagUseredit = '7',
  FlagAnonymous = '8',
  FlagNuke = 'A',
  FlagUnnuke = 'B',
  FlagUndupe = 'C',
  FlagKick = 'D',
  FlagKill = 'E',
  FlagTake = 'F',
  FlagGive = 'G',
  FlagUsers = 'H',
  FlagIdler = 'I',
  FlagCustom1 = 'J',
  FlagCustom2 = 'K',
  FlagCustom3 = 'L',
  FlagCustom4 = 'M',
  FlagCustom6 = 'N'
};

} /* acl namespace */

#ifndef __ACL_FLAGS_CPP
extern const std::string validFlags;
#endif

#endif
