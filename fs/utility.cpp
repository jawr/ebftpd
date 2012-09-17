#include <unistd.h>
#include "utility.hpp"

namespace fs
{

bool IsFile(const std::string& path)
{
  return !access(path.c_str(), F_OK);
}

bool IsDirectory(const std::string& path);
bool IsLink(const std::string& path);


} /* fs namespace */