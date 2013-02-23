#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cmd/site/search.hpp"
#include "db/index/index.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "fs/directory.hpp"
#include "cfg/get.hpp"
#include "cmd/error.hpp"
#include "util/string.hpp"

namespace cmd { namespace site
{

void SEARCHCommand::Execute()
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
  auto results = db::index::Search(terms, number);
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
      long long kBytes;
      auto e = fs::DirectorySize(fs::MakeReal(fs::VirtualPath(result.path)),
                                 cfg::Get().DirSizeDepth(), kBytes);
      if (e.Errno() == ENOENT)
      {
        db::index::Delete(result.path);
        continue;
      }

      body.RegisterValue("index", ++index);
      body.RegisterValue("datetime", boost::lexical_cast<std::string>(result.dateTime));
      body.RegisterValue("path", result.path);
      body.RegisterValue("section", result.section);
      body.RegisterSize("size", e ? kBytes : -1);
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
