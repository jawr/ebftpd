#include "text/factory.hpp"
#include "text/error.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "fs/diriterator.hpp"
#include "fs/path.hpp"
#include "logs/logs.hpp"

namespace text
{
boost::mutex Factory::mutex;
std::unique_ptr<Factory> Factory::instance;

util::Error Factory::Initalize()
{
  std::unique_ptr<Factory> factory(new Factory());
  
  fs::Path textpath = cfg::Get().Datapath() / "text";
  
  try
  {
    fs::DirIterator it(textpath);
    fs::DirIterator end;
   
    // let's get all error's in one go 
    int errors = 0;

    for (; it != end; ++it)
    {
      if (it->Extension() != "tmpl") continue;
      
      fs::Path file(textpath / *it);
      std::string name = it->NoExtension(); 
      boost::to_lower(name);

      try
      {
        TemplateParser templ(file.ToString());
        factory->templates.insert(std::make_pair(name, templ.Create()));
      }
      catch (const text::TemplateError& e)
      {
        logs::error << "Template Initalize error (" << *it << "): " << e.Message() << logs::endl;
        ++errors;
      }
    }
    if (errors > 0)
    {
      std::ostringstream os;
      os << errors << " template errors. Please review errors above.";
      throw text::TemplateError(os.str());
    }
  }
  catch (const util::SystemError& e)
  {
    logs::error << e.Message() << logs::endl;
    return util::Error::Failure(e.Message());
  }

  boost::lock_guard<boost::mutex> lock(mutex);
  instance = std::move(factory);
  return util::Error::Success();
}

Template Factory::GetTemplate(const std::string& templ)
{
  std::string name = boost::to_lower_copy(templ);
  
  boost::lock_guard<boost::mutex> lock(mutex);
  if (instance->templates.find(name) == instance->templates.end())
    throw TemplateError("No such template (" + templ + ")");
  return instance->templates.at(name);
}
  
// end
}
