#ifndef __TEXT_UTIL_HPP
#define __TEXT_UTIL_HPP

#include <string>
#include "util/error.hpp"

namespace fs
{
class Path;
}

namespace ftp
{
class Client;
}

namespace text
{
class TemplateSection;
class Template;

void RegisterGlobals(const ftp::Client& client, TemplateSection& ts);
util::Error GenericTemplate(const ftp::Client& client, Template& tmpl, 
      std::string& messages);
util::Error GenericTemplate(const ftp::Client& client, const std::string& name, 
      std::string& messages);
util::Error GenericTemplate(const ftp::Client& client, const fs::Path& path,
      std::string& messages);
      
} /* text namespace */

#endif
