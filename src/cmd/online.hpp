#ifndef __CMD_ONLINE_HPP
#define __CMD_ONLINE_HPP

#include <string>

namespace text
{
class Template;
}

namespace cmd
{

std::string CompileWhosOnline(text::Template& templ);
std::string CompileWhosOnline(const std::string& id, text::Template& templ);

} /* cmd namespace */

#endif
