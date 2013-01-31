#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/search.hpp"
#include "db/index/index.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"

namespace cmd { namespace site
{

void SEARCHCommand::Execute()
{
  auto results = db::index::Search(std::vector<std::string>(args.begin() + 1, args.end()));
  if (results.empty()) control.Reply(ftp::CommandOkay, "No search results.");
  else
  {
    boost::optional<text::Template> templ;
    try
    {
      templ.reset(text::Factory::GetTemplate("search"));
    }
    catch (const text::TemplateError& e)
    {
      control.Reply(ftp::ActionNotOkay, e.Message());
      return;
    }
    
    std::ostringstream os;
    os << templ->Head().Compile();

    text::TemplateSection& body = templ->Body();

    unsigned index = 0;
    for (const auto& result : results)
    {
      body.RegisterValue("index", ++index);
      body.RegisterValue("datetime", boost::lexical_cast<std::string>(result.dateTime));
      body.RegisterValue("path", result.path);
      body.RegisterValue("section", result.section);
      os << body.Compile();
    }

    text::TemplateSection& foot = templ->Foot();
    foot.RegisterValue("count", results.size());
    os << foot.Compile();
    
    control.Reply(ftp::CommandOkay, os.str());
  }
}

} /* site namespace */
} /* cmd namespace */
