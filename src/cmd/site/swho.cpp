#include "cmd/site/swho.hpp"
#include "cfg/get.hpp"
#include "text/error.hpp"
#include "text/factory.hpp"
#include "cmd/online.hpp"

namespace cmd { namespace site
{

void SWHOCommand::Execute()
{
  boost::optional<text::Template> templ;
  try
  {
    templ.reset(text::Factory::GetTemplate("swho"));
  }
  catch (const text::TemplateError& e)
  {
    control.Reply(ftp::ActionNotOkay, e.Message());
    return;
  }

  control.Reply(ftp::CommandOkay, cmd::CompileWhosOnline(*templ));
}

// end
}
}
