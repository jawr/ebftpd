#include <sstream>
#include <iostream>
#include "util/lrucache.hpp"

int main()
{
    using namespace util;
    
    LRUCache<int, std::string> cache(2);
    
    for (int i = 0; i < 10000; ++i)
    {
      std::ostringstream os;
      os << i;
      cache.Insert(i, os.str());
    }
    
    std::cout << "iterate" << std::endl;
    
    for (LRUCache<int, std::string>::const_iterator it =
         cache.begin(); it != cache.end(); ++it)
    {
      std::cout << it->first << std::endl;
    }
}
