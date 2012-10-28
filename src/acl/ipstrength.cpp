#ifdef ACL_IPSTRENGTH_TEST

#include "acl/ipstrength.hpp"

void Display(const acl::IPStrength& ips)
{
  std::cout << ips.NumOctets() << " " << ips.IsHostname() << " " << ips.HasIdent() << std::endl;
}

int main()
{
  using namespace acl;
  
  IPStrength ips1(3, true, false);
  
  IPStrength ips2("*@127aaaa.0.0.*");
  
  Display(ips2);
  
  std::cout << ips1.Allowed(ips2) << std::endl;
  
  
}

#endif