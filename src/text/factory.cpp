#include "text/factory.hpp"
#include "text/error.hpp"
#include "cfg/config.hpp"
#include "cfg/get.hpp"
#include "fs/diriterator.hpp"
#include "fs/path.hpp"
#include "logs/logs.hpp"

namespace text
{
std::mutex Factory::mutex;
std::unique_ptr<Factory> Factory::instance;

util::Error Factory::Initialize()
{
  std::unique_ptr<Factory> factory(new Factory());
  
  auto textpath = fs::Path(cfg::Get().Datapath()) / "text";
  
  try
  {
    util::path::DirIterator it(textpath.ToString());
    util::path::DirIterator end;
   
    // let's get all error's in one go 
    int errors = 0;

    for (; it != end; ++it)
    {
      fs::Path p(*it);
      if (p.Extension() != "tmpl") continue;
      
      fs::Path file(textpath / p);
      std::string name = p.NoExtension(); 
      boost::to_lower(name);

      try
      {
        TemplateParser templ(file.ToString());
        factory->templates.insert(std::make_pair(name, templ.Create()));
      }
      catch (const text::TemplateError& e)
      {
        logs::Error("Template Initialize error (%1%): %2%", *it, e.Message());
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
    logs::Error("Unable to open template directory: %1%", e.Message());
    return util::Error::Failure(e.Message());
  }

  std::lock_guard<std::mutex> lock(mutex);
  instance = std::move(factory);
  return util::Error::Success();
}

Template Factory::GetTemplate(const std::string& templ)
{
  std::string name = boost::to_lower_copy(templ);
  
  std::lock_guard<std::mutex> lock(mutex);
  if (!instance.get() || instance->templates.find(name) == instance->templates.end())
    throw TemplateError("No such template (" + templ + ")");
  return instance->templates.at(name);
}
  
// end
}
