#include "ftp/addrallocator.hpp"

namespace ftp
{


}

#ifdef FTP_ADDRALLOCATOR_TEST

#include <iostream>

int main()
{
  using namespace ftp;
  
  std::vector<std::string> addresses;
  addresses.push_back("A");
  addresses.push_back("B");
  addresses.push_back("C");
  
  AddrAllocator<AddrType::Active>::SetAddrs(addresses);
  for (int i = 0; i < 300; ++i)
      std::cout << AddrAllocator<AddrType::Active>::NextAddr() << std::endl;
      
  AddrAllocator<AddrType::Passive>::SetAddrs(addresses);
  for (int i = 0; i < 300; ++i)
      std::cout << AddrAllocator<AddrType::Passive>::NextAddr() << std::endl;

}

#endif
