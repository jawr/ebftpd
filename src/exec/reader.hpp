#ifndef __EXEC_READER_HPP
#define __EXEC_READER_HPP

#include <string>
#include <vector>

namespace ftp
{
class Client;
}

namespace exec
{

class Reader
{
  ftp::Client& client;
  bool open;
  std::string file;
  std::vector<std::string> argv;
  
public:
  Reader(ftp::Client& client, const std::vector<std::string>& argv);
      
  ~Reader();
      
  bool Getline(std::string& line);
  void Close();
};

} /* exec namespace */

#endif
