#ifdef UTIL_REVERSEL_LOG_READER_TEST

#include <iostream>
#include "reverselogreader.hpp"

int main(int argc, char** argv)
{
	util::ReverseLogReader log(argv[1]);
  
  std::string line;
  while (log.Getline(line))
  {
    std::cout << line << std::endl;
  }
}

#endif
