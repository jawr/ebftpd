#ifndef __FS_STATUS_HPP
#define __FS_STATUS_HPP

#include <string>
#include <sys/stat.h>

namespace fs
{

class Status
{
  std::string path;
  struct stat native;
  bool statOkay;
  
  Status& Check();
  
public:
  Status() : statOkay(false) { }
  Status(const std::string& path);
  
  Status& Check(const std::string& path);
  
  bool IsRegularFile();
  bool IsDirectory();
  bool IsLink();
  
  off_t Size();
  
  const struct stat& Native();
};

} /* fs namespace */

#endif
