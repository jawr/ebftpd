#ifndef __TEXT_TEMPLATE_HPP
#define __TEXT_TEMPLATE_HPP

#include <string>
#include <unordered_map>

#include "text/tag.hpp"

namespace text
{

class Template
{
  std::string file;
  std::unordered_map<std::string, Tag> tags;
public:
  Template(const std::string& file);
  void Register(std::string var);
};

// end
}
#endif

