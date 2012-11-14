#ifndef __TEXT_FACTORY_HPP
#define __TEXT_FACTORY_HPP

#include <string>
#include <unordered_map>
#include "text/template.hpp"
#include "util/error.hpp"

namespace text
{

class Factory
{
  std::unordered_map<std::string, Template> templates;
  static Factory instance;
  Factory() {}
public:
  
  static util::Error Initalize();
  static int Size() { return instance.templates.size(); }
  static Template GetTemplate(const std::string& templ);
};
  
// end
}
#endif
