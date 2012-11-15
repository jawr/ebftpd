#ifndef __EXEC_READER_HPP
#define __EXEC_READER_HPP

#include <string>
#include <vector>

namespace util
{
class ProcessReader;
}

namespace exec
{

class Reader
{
  util::ProcessReader& child;
  bool open;
  
public:
  Reader(util::ProcessReader& child, const std::string& file, 
      std::vector<std::string> argv);
      
  ~Reader();
      
  bool Getline(std::string& line);
  void Close();
};

} /* exec namespace */

#endif
