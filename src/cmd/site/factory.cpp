#include <boost/algorithm/string.hpp>
#include "cmd/site/factory.hpp"
#include "cmd/site/epsv.hpp"
#include "cmd/site/idle.hpp"
#include "cmd/site/vers.hpp"
#include "cmd/site/xdupe.hpp"
#include "cmd/site/passwd.hpp"
#include "cmd/site/chpass.hpp"
#include "cmd/site/deluser.hpp"
#include "cmd/site/readd.hpp"
#include "cmd/site/purge.hpp"
#include "cmd/site/renuser.hpp"
#include "cmd/site/adduser.hpp"
#include "cmd/site/gadduser.hpp"
#include "cmd/site/addip.hpp"
#include "cmd/site/flags.hpp"
#include "cmd/site/delip.hpp"
#include "cmd/site/help.hpp"
#include "cmd/site/tagline.hpp"
#include "cmd/site/chmod.hpp"
#include "cmd/site/chown.hpp"
#include "cmd/site/logs.hpp"
#include "cmd/site/wipe.hpp"
#include "cmd/site/time.hpp"
#include "cmd/site/grpadd.hpp"
#include "cmd/site/chgrp.hpp"
#include "cmd/site/change.hpp"
#include "cmd/site/user.hpp"
#include "cmd/site/users.hpp"
#include "cmd/site/seen.hpp"
#include "cmd/site/setpgrp.hpp"
#include "cmd/site/kick.hpp"
#include "cmd/site/who.hpp"
#include "cmd/site/swho.hpp"
#include "cmd/site/give.hpp"
#include "cmd/site/take.hpp"
#include "cmd/site/stats.hpp"
#include "cmd/site/group.hpp"
#include "cmd/site/groups.hpp"
#include "cmd/site/grpdel.hpp"
#include "cmd/site/grpchange.hpp"
#include "cmd/site/grpnfo.hpp"
#include "cmd/site/grpren.hpp"
#include "cmd/site/reload.hpp"
#include "cmd/site/shutdown.hpp"
#include "cmd/site/sreply.hpp"
#include "cmd/site/msg.hpp"
#include "cmd/site/traffic.hpp"
#include "cmd/site/customcommand.hpp"
#include "cfg/get.hpp"
#include "util/verify.hpp"
#include "cmd/site/grpchange.hpp"

namespace cmd { namespace site
{

Factory Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "EPSV",       { 0,  1,  "epsv",
                      CreatorBasePtr(new Creator<site::EPSVCommand>()),  
                      "Syntax: SITE EPSV normal|full",
                      "Change EPSV command mode between normal and full (fxp support)"  }, },
    { "IDLE",       { 0,  1,  "idle",
                      CreatorBasePtr(new Creator<site::IDLECommand>()),
                      "Syntax: SITE IDLE [<seconds>]",
                      "Changes idle timeout" }, },
    { "VERS",       { 0,  0,  "vers",
                      CreatorBasePtr(new Creator<site::VERSCommand>()),
                      "Syntax: SITE VERS",
                      "Display server version" }, },
    { "XDUPE",      { 0,  1,  "xdupe",
                      CreatorBasePtr(new Creator<site::XDUPECommand>()),
                      "Syntax: SITE XDUPE [<mode>]",
                      "Display / change extended dupe mode" }, },
    { "PASSWD",     { 1,  1,  "passwd",
                      CreatorBasePtr(new Creator<site::PASSWDCommand>()),
                      "Syntax: SITE PASSWD <password>",
                      "Change your password" }, },
    { "CHPASS",     { 2,  2,  "chpass",
                      CreatorBasePtr(new Creator<site::CHPASSCommand>()),
                      "Syntax: SITE CHPASS <user> <password>",
                      "Change another user's password" }, },
    { "DELUSER",    { 1,  1,  "deluser",
                      CreatorBasePtr(new Creator<site::DELUSERCommand>()),
                      "Syntax: SITE DELUSER <user>",
                      "Delete a user" }, },
    { "READD",      { 1,  1,  "readd",
                      CreatorBasePtr(new Creator<site::READDCommand>()),
                      "Syntax: SITE READD <user>",
                      "Readd a deleted user" }, },
    { "PURGE",      { 1,  1,  "purge",
                      CreatorBasePtr(new Creator<site::PURGECommand>()),
                      "Syntax: SITE PURGE <user>",
                      "Purge a deleted user" }, },
    { "RENUSER",    { 2,  2,  "renuser",
                      CreatorBasePtr(new Creator<site::RENUSERCommand>()),
                      "Syntax: SITE RENUSER <old user> <new user>",
                      "Rename a user" }, },
    { "ADDUSER",    { 2, -1,  "adduser",
                      CreatorBasePtr(new Creator<site::ADDUSERCommand>()),
                      "Syntax: SITE ADDUSER <user> <password> [<ident@ip>..]",
                      "Add new user" }, },
    { "TADDUSER",   { 3, -1,  "tadduser",
                      nullptr,
                      "Syntax: SITE TADDUSER <template> <user> <password> [<ident@ip>..]",
                      "Add new user based on a template" }, },
    { "ADDIP",      { 2, -1,  "addip|addownip",
                      CreatorBasePtr(new Creator<site::ADDIPCommand>()),
                      "Syntax: SITE ADDIP <user> <ident@ip> [<ident@ip>..]",
                      "Add ident@ip to user" }, },
    { "FLAGS",      { 0, 1, "flags|flagsown",
                      CreatorBasePtr(new Creator<site::FLAGSCommand>()),
                      "Syntax: SITE FLAGS [<user>]",
                      "View your own or another user's flags" }, },
    { "DELIP",      { 2, -1,  "delip|delownip",
                      CreatorBasePtr(new Creator<site::DELIPCommand>()),
                      "Syntax: SITE DELIP <user> <ident@ip> [<ident@ip>..]",
                      "Delete ident@ip from user" }, },
    { "CHANGE",     { 3,  3,  "change|changeallot|changeflags|"
                      "changeratio|changesratio|changehomedir", 
                      CreatorBasePtr(new Creator<site::CHANGECommand>()),  
                      "Syntax: SITE CHANGE <user> <setting> <value>\n"
                      "        SITE CHANGE {<user> [<user> ..]} <setting> <value>\n"
                      "        SITE CHANGE * <setting> <value>",
                      "Change settings for a user or users" }, },
    { "KICK",       { 1,  1,  "kick",
                      CreatorBasePtr(new Creator<site::KICKCommand>()),
                      "Syntax: SITE KICK <user>",
                      "Kick a user's connections from the site" }, },
    { "SEEN",       { 1,  1,  "seen",
                      CreatorBasePtr(new Creator<site::SEENCommand>()),
                      "Syntax: SITE SEEN <user>",
                      "Display last time a user logged in" }, },
    { "USERS",      { 0,  -1, "users",
                      CreatorBasePtr(new Creator<site::USERSCommand>()),  
                      "Syntax: SITE USERS [<criteria> ..]",
                      "Detailed list of users" }, },
    { "GIVE",       { 2,  3,  "give|giveown",
                      CreatorBasePtr(new Creator<site::GIVECommand>()),
                      "Syntax: SITE GIVE <user> <credits>G|M [<message>]",
                      "Give credits to user" }, },
    { "TAKE",       { 2, 3,   "take",
                      CreatorBasePtr(new Creator<site::TAKECommand>()),
                      "Syntax: SITE TAKE <user> <credits>G|M [<message>]",
                      "Take credits from user" }, },
    { "STATS",      { 0,  1,  "stats|statsown",
                      CreatorBasePtr(new Creator<site::STATSCommand>()),
                      "Syntax: SITE STATS [<user>]",
                      "Display user's transfer stats" }, },
    { "USER",       { 0,  1,  "user|userown",
                      CreatorBasePtr(new Creator<site::USERCommand>()),
                      "Syntax: SITE USER [<user>]",
                      "Display user's info" }, },
    { "TAGLINE",    { 1, -1,  "tagline",
                      CreatorBasePtr(new Creator<site::TAGLINECommand>()),
                      "Syntax: SITE TAGLINE <tagline>",
                      "Change your tagline" }, },
    { "SETPGRP",    { 2,  2,  "setpgrp",
                      CreatorBasePtr(new Creator<site::SETPGRPCommand>()),
                      "Syntax: SITE SETPGRP <user> <group>",
                      "Set's a user's primary group" }, },
    { "CHGRP",      { 2, -1,  "chgrp",
                      CreatorBasePtr(new Creator<site::CHGRPCommand>()),
                      "Syntax: SITE CHGRP <user> [+|-|=] <group> [<group> ..]",
                      "Change user's groups" }, },
    { "RANKS",      { 3, -1,  "ranks",
                      nullptr,
                      "Syntax: SITE RANKS DAY|WEEK|MONTH|ALL UP|DOWN SPEED|AMOUNT|FILES [<number>] [<section>] [=<group>]",
                      "Display user upload/download rankings" }, },
    { "GRPRANKS",   { 3, -1,  "grpranks",
                      nullptr,
                      "Syntax: SITE GRPRANKS DAY|WEEK|MONTH|ALL UP|DOWN SPEED|AMOUNT|FILES [<number>] [<section>]",
                      "Display group upload/download rankings" }, },
    { "NUKE",       { 3,  -1, "nuke",
                      nullptr,
                      "Syntax: SITE NUKE <path> <multiplier> <message>",
                      "Nuke a directory" }, },
    { "UNNUKE",     { 2, -1, "unnuke",
                      nullptr,
                      "Syntax: SITE UNNUKE <path> <message>",
                      "Unnuke a directory" }, },
    { "NUKES",      { 0,  2,  "nukes",
                      nullptr,
                      "Syntax: SITE NUKES [<number>] [<section>]",
                      "Display nuke history" }, },
    { "UNNUKES",    { 0,  2,  "unnukes",
                      nullptr,
                      "Syntax: SITE UNNUKES [<number>] [<section>]",
                      "Display unnuke history" }, },
    { "UNDUPE",     { 1,  1,  "undupe",
                      nullptr,
                      "Syntax: SITE UNDUPE <filemask>",
                      "Undupe files matching a mask" }, },
    { "PREDUPE",    { 1,  1,  "predupe",
                      nullptr,
                      "Syntax: SITE PREDUPE <filemask>",
                      "Forcefully dupe all future uploads matching a file mask" }, },
    { "UPDATE",     { 1,  1,  "update",
                      nullptr,
                      "Syntax: SITE UPDATE <pathmask>",
                      "Add all directories matching a path mask to the site search index" }, },
    { "DUPE",       { 1,  -1, "dupe",
                      nullptr,
                      "Syntax: SITE DUPE [-MAX <number>] [-FROM <mmddyy>] [-TO <mmddyy>] <string> [<string> .. ]",
                      "Search the dupe database for directories" }, },
    { "FDUPE",      { 1,  -1, "fdupe",
                      nullptr,
                      "Syntax: SITE FDUPE [-MAX <number>] [-FROM <mmddyy>] [-TO <mmddyy>] <string> [<string> .. ]",
                      "Search the dupe database for files" }, },
    { "NEW",        { 0,  -1,  "new",
                      nullptr,
                      "Syntax: SITE NEW [<number>] [<path>|<section>]",
                      "Display latest directories" }, },
    { "CHOWN",      { 2,  -1,  "chown",
                      CreatorBasePtr(new Creator<site::CHOWNCommand>()),
                      "Syntax: SITE CHOWN [-R] <owner> <pathmask>",
                      "Change file and/or directory ownership" }, },
    { "CHMOD",      { 2,  -1,  "chmod",
                      CreatorBasePtr(new Creator<site::CHMODCommand>()),
                      "Syntax: SITE CHMOD [-R] <symbolic-mode|octal-mode> <pathmask>",
                      "Change file and/or directory permissions" }, },
    { "EMULATE",    { 1,  1,  "emulate",
                      nullptr,
                      "Syntax: SITE EMULATE <user>",
                      "Become another user by temporarily loading their userfile" }, },
    { "TRAFFIC",    { 0,  0,  "traffic",
                      CreatorBasePtr(new Creator<site::TRAFFICCommand>()),
                      "Syntax: SITE TRAFFIC",
                      "Display traffic statistics" }, },
    { "WHO",        { 0,  0,  "who",
                      CreatorBasePtr(new Creator<site::WHOCommand>()),
                      "Syntax: SITE WHO",
                      "Display who's online" }, },
    { "SWHO",       { 0,  0,  "swho",
                      CreatorBasePtr(new Creator<site::SWHOCommand>()),
                      "Syntax: SITE SWHO",
                      "Display detailed who's online" }, },
    { "WIPE",       { 1,  -1,  "wipe",
                      CreatorBasePtr(new Creator<site::WIPECommand>()),
                      "Syntax: SITE WIPE [-R] <pathmask>",
                      "Wipe a file or directory (and it's contents)" }, },
    { "LOGS",       { 1,  -1, "logs|errorlog|securitylog|eventlog|dblog|siteoplog|debuglog",
                      CreatorBasePtr(new Creator<site::LOGSCommand>()),
                      "Syntax: SITE -[-MAX <number>] LOGS ERROR|SECURITY|EVENTS|DB|SITEOP|DEBUG [<string> ..]",
                      "Display entries from a system log" }, },
    { "LASTON",     { 0,  -1, "laston",
                      nullptr,
                      "Syntax: SITE LASTON [<number>] [<criteria> ..]",
                      "Display last users online" }, },
    { "ONEL",       { 1,  -1, "onel|oneladd",
                      nullptr,
                      "Syntax: SITE ONEL [-MAX <number>|<message> [message> ..]]",
                      "Display or add  to the onliners" }, },
    { "CHAGMIN",    { 2,  2,  "chgadmin",
                      nullptr,
                      "Syntax: SITE CHGADMIN <user> <group",
                      "Toggle gadmin status for a user on the specified group" }, },
    { "GADDUSER",   { 3,  -1, "gadduser",
                      CreatorBasePtr(new Creator<site::GADDUSERCommand>()),
                      "Syntax: SITE GADDUSER <group> <user> <password> [<ident@ip> ..]",
                      "Add a user directly to a group" }, },
    { "GROUPS",     { 0,  0,  "groups",
                      CreatorBasePtr(new Creator<site::GROUPSCommand>()),
                      "Syntax: SITE GROUPS",
                      "Display a list of all groups" }, },
    { "GROUP",      { 0,  1,  "group",
                      CreatorBasePtr(new Creator<site::GROUPCommand>()),
                      "Syntax: SITE GROUP",
                      "Display detailed group info" }, },
    { "GRPCHANGE",  { 3,  3,  "grpchange", 
                      CreatorBasePtr(new Creator<site::GRPCHANGECommand>()),
                      "Syntax: SITE GRPCHANGE <group> <setting> <value>\n"
                      "        SITE GRPCHANGE {<group> [<group> ..]} <setting> <value>\n"
                      "        SITE GRPCHANGE * <setting> <value>",
                      "Change settings for a group or groups" }, },
    { "GRPADD",     { 1, -1,  "grpadd",
                      CreatorBasePtr(new Creator<site::GRPADDCommand>()),
                      "Syntax: SITE GRPADD <group> [<tagline>]",
                      "Add a group" }, },
    { "GRPDEL",     { 1,  1,  "grpdel",
                      CreatorBasePtr(new Creator<site::GRPDELCommand>()),
                      "Syntax: SITE GRPDEL <group>",
                      "Delete a group" }, },
    { "GRPREN",     { 2,  2,  "grpren",
                      CreatorBasePtr(new Creator<site::GRPRENCommand>()),
                      "Syntax: SITE GRPREN <old group> <new group>",
                      "Rename a group" }, },
    { "GRPNFO",     { 2,  -1, "grpnfo",
                      CreatorBasePtr(new Creator<site::GRPNFOCommand>()),
                      "Syntax: SITE GRPNFO <group> <tagline>",
                      "Change group description" }, },
    { "HELP",       { 0,  1,  "help",
                      CreatorBasePtr(new Creator<site::HELPCommand>()),
                      "Syntax: SITE HELP [<command>]",
                      "Display site command help" }, },
    { "STAT",       { 0,  0,  "stat",
                      nullptr,
                      "Syntax: SITE STAT",
                      "Display statline" }, },
    { "TIME",       { 0,  0,  "time",
                      CreatorBasePtr(new Creator<site::TIMECommand>()),
                      "Syntax: SITE TIME",
                      "Display local time" }, },
    { "SEARCH",     { 1,  -1, "search",
                      nullptr,
                      "Syntax: SITE SEARCH [-MAX <number>] [-SECTION <section>] <string> [<string> ..]",
                      "Search the site index" }, },
    { "WELCOME",    { 0,  0,  "welcome",
                      nullptr,
                      "Syntax: SITE WELCOME",
                      "Display welcome message" }, },
    { "GOODBYE",    { 0,  0,  "goodbye",
                      nullptr,
                      "Syntax: SITE GOODBYE",
                      "Display goodbye message" }, },
    { "MSG",        { 1,  -1, "msg|msg*|msg=|msg{",
                      CreatorBasePtr(new Creator<site::MSGCommand>()),
                      "Syntax: SITE MSG <command> [<arguments> ..]\n"
                      "Commands: READ [<index>]                     - Read message(s)\n"
                      "          SEND <user>|=<group> <message>     - Send message to user or group\n"
                      "          SEND { <user> =<group> } <message> - Send message to multiple users or groups\n"
                      "          SEND * <message>                   - Send message to all users\n"
                      "          LIST                               - List messages\n"
                      "          SAVE [<index>]                     - Mark message(s) as saved\n"
                      "          PURGE [<index>]                    - Purge a message(s)",
                      "Messaging system" }, },
    { "REQUEST",    { 1,  1,  "request",
                      nullptr,
                      "Syntax: SITE REQUEST <string>",
                      "Add a request" }, },
    { "REQFILLED",  { 1,  1,  "reqfilled",
                      nullptr,
                      "Syntax: SITE REQFILLED <number>",
                      "Mark a request as filled" }, },
    { "REQUESTS",   { 0,  2,  "requests",
                      nullptr,
                      "Syntax: SITE REQUESTS [<number>] [<string> ..]",
                      "Display list of requests" }, },
    { "RELOAD",     { 0,  0,  "reload",
                      CreatorBasePtr(new Creator<site::RELOADCommand>()),
                      "Syntax: SITE RELOAD",
                      "Reload config file" }, },
    { "SHUTDOWN",   { 1,  1,  "shutdownfull|shutdownsiteop",
                      CreatorBasePtr(new Creator<site::SHUTDOWNCommand>()),
                      "Syntax: SITE SHUTDOWN SITEOP|REOPEN|FULL",
                      "Shutdown to siteop only or full shutdown and exit" }, },
    { "SREPLY",     { 0,  1,  "sreply",
                      CreatorBasePtr(new Creator<site::SREPLYCommand>()),
                      "Syntax: SITE SREPLY [ON|OFF]",
                      "Turn single line replies on and off" }, }
  };
}

CommandDefOpt Factory::LookupCustom(const std::string& command)
{
  const cfg::setting::SiteCmd* match = nullptr;
  for (auto& siteCmd : cfg::Get().SiteCmd())
  {
    if (siteCmd.Command() == command)
    {
      match = &siteCmd;
      break;
    }
  }
  
  if (!match) return CommandDefOpt();
  
  CommandDefOpt def;
  std::string aclKeyword("custom-" + command);
  switch (match->GetType())
  {
    case cfg::setting::SiteCmd::Type::EXEC  :
    {
      def.reset(CommandDef(boost::to_lower_copy(aclKeyword), 
          CreatorBasePtr(new CustomCreator<site::CustomEXECCommand>(*match))));
      break;
    }
    case cfg::setting::SiteCmd::Type::TEXT  :
    {
      def.reset(CommandDef(boost::to_lower_copy(aclKeyword), 
          CreatorBasePtr(new CustomCreator<site::CustomTEXTCommand>(*match))));
      break;
    }
    case cfg::setting::SiteCmd::Type::ALIAS :
    {
      def.reset(CommandDef(boost::to_lower_copy(aclKeyword), 
          CreatorBasePtr(new CustomCreator<site::CustomALIASCommand>(*match))));
      break;
    }
    default                                 :
    {
      verify(false);
    }
  }
  
  return def;
}

CommandDefOpt Factory::Lookup(const std::string& command, bool noCustom)
{
  CommandDefOpt def;
  if (!noCustom) def = LookupCustom(command);
  if (!def)
  {
    CommandDefsMap::const_iterator it = factory.defs.find(command);
    if (it != factory.defs.end()) def.reset(it->second);
  }
  return def;
}

std::unordered_set<std::string> Factory::ACLKeywords()
{
  std::unordered_set<std::string> keywords;
  for (auto& kv : Commands())
  {
    std::vector<std::string> curKeywords;
    boost::split(curKeywords, kv.second.ACLKeyword(), boost::is_any_of("|"));
    keywords.insert(curKeywords.begin(), curKeywords.end());
  }
  
  return keywords;
}

} /* site namespace */
} /* cmd namespace */

#ifdef CMD_SITE_FACTORY_TEST

#include <iostream>
#include <memory>
#include "ftp/client.hpp"

int main()
{
  using namespace cmd;
  
  ftp::ClientState::Client client;
  Args args;
  args.push_back("USER");
  std::unique_ptr<cmd::Command> cmd(Factory::Create(client, args));
  cmd->Execute();
  args.clear();
  args.push_back("PASS");
  cmd.reset(Factory::Create(client, args));
  cmd->Execute();
}

#endif
