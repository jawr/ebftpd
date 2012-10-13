#include "ftp/portallocator.hpp"

namespace ftp
{


}

#ifdef FTP_PORTALLOCATOR_TEST

#include <iostream>



int main()
{
  using namespace ftp;
  
  std::vector<std::string> toks;
  toks.push_back("10000-10100");
  toks.push_back("20000-20100");
  toks.push_back("33333");
  cfg::setting::Ports ports(toks);
  
  PortAllocator<PortType::Active>::SetPorts(ports);
  for (int i = 0; i < 300; ++i)
      std::cout << PortAllocator<PortType::Active>::NextPort() << std::endl;
      
  PortAllocator<PortType::Passive>::SetPorts(ports);
  for (int i = 0; i < 300; ++i)
      std::cout << PortAllocator<PortType::Passive>::NextPort() << std::endl;

}

#endif
