#ifndef __TEXT_FACTORY_HPP
#define __TEXT_FACTORY_HPP

#include <string>
#include <unordered_map>
#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include "text/template.hpp"
#include "text/parser.hpp"
#include "text/templatesection.hpp"
#include "util/error.hpp"

namespace text
{

class Factory
{
  std::unordered_map<std::string, Template> templates;
  
  static boost::mutex mutex;
  static std::unique_ptr<Factory> instance;

  Factory() = default;

public:
  
  static util::Error Initialize();
  static int Size()
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    return instance->templates.size();
  }
  static Template GetTemplate(const std::string& templ);
};
  
// end
}
#endif
