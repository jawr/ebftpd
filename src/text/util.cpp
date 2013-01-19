#include <sstream>
#include "text/util.hpp"
#include "fs/path.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"

namespace text
{

util::Error GenericTemplate(const std::string& name, std::string& messages)
{
  try
  {
    Template tmpl = Factory::GetTemplate(name);
    
    // insert other tags ???
    
    std::ostringstream os;
    os << tmpl.Head().Compile();
    os << tmpl.Body().Compile();
    os << tmpl.Foot().Compile();
    
    messages = os.str();
  }
  catch (const TemplateError& e)
  {
    return util::Error::Failure(e.Message());
  }
  
  return util::Error::Success();
}

} /* text namespace */
