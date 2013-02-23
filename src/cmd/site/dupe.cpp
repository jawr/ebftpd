#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/dupe.hpp"
#include "db/dupe/dupe.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "cmd/error.hpp"
#include "util/string.hpp"

namespace cmd { namespace site
{

void DUPECommand::Execute()
{
  int number = 10;
  unsigned termsOffset = 1;
  if (util::ToLowerCopy(args[1]) == "-max")
  {
    if (args.size() < 4) throw cmd::SyntaxError();
    
    try
    {
      number = boost::lexical_cast<int>(args[2]);
      if (number <= 0) throw boost::bad_lexical_cast();
    }
    catch (const boost::bad_lexical_cast&)
    {
      throw cmd::SyntaxError();
    }
    
    termsOffset += 2;
  }

  std::vector<std::string> terms(args.begin() + termsOffset, args.end());
  auto results = db::dupe::Search(terms, number);
  if (results.empty()) control.Reply(ftp::CommandOkay, "No dupe results.");
  else
  {
    boost::optional<text::Template> templ;
    try
    {
      templ.reset(text::Factory::GetTemplate("dupe"));
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
      body.RegisterValue("date", boost::posix_time::to_iso_string(result.dateTime).substr(0, 8));
      body.RegisterValue("directory", result.directory);
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
