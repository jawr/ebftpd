#ifdef FS_DIRCONTAINER_TEST

#include "fs/dircontainer.hpp"

#include <iostream>

int main()
{
	for (auto& name : fs::DirContainer("."))
  {
    std::cout << "omg! " << name << std::endl;
  }
}

#endif
