#ifndef __TEXT_FACTORY_HPP
#define __TEXT_FACTORY_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "text/template.hpp"
#include "text/parser.hpp"
#include "text/templatesection.hpp"
#include "util/error.hpp"

namespace text
{

class Factory
{
  std::unordered_map<std::string, Template> templates;
  
  static std::mutex mutex;
  static std::unique_ptr<Factory> instance;

  Factory() = default;

public:
  
  static util::Error Initialize();
  static int Size()
  {
    std::lock_guard<std::mutex> lock(mutex);
    return instance->templates.size();
  }
  static Template GetTemplate(const std::string& templ);
};
  
// end
}
#endif
