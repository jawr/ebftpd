#include "cmd/site/factory.hpp"
#include "cfg/get.hpp"
#include "util/verify.hpp"
#include "cmd/site/commands.hpp"
#include "cmd/site/adduser.hpp"
#include "cmd/site/help.hpp"
#include "cmd/site/chmod.hpp"
#include "cmd/site/chown.hpp"
#include "cmd/site/logs.hpp"
#include "cmd/site/wipe.hpp"
#include "cmd/site/change.hpp"
#include "cmd/site/group.hpp"
#include "cmd/site/msg.hpp"
#include "cmd/site/customcommand.hpp"
#include "cmd/site/grpchange.hpp"

namespace cmd { namespace site
{

std::unique_ptr<Factory> Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "IDLE",       { 0,  1,  "idle",
                      std::make_shared<Creator<IDLECommand>>(),
                      "Syntax: SITE IDLE [<seconds>]",
                      "Changes idle timeout" }, },
    { "VERS",       { 0,  0,  "vers",
                      std::make_shared<Creator<VERSCommand>>(),
                      "Syntax: SITE VERS",
                      "Display server version" }, },
    { "XDUPE",      { 0,  1,  "xdupe",
                      std::make_shared<Creator<XDUPECommand>>(),
                      "Syntax: SITE XDUPE 0|1|2|3|4",
                      "Display / change extended dupe mode" }, },
    { "PASSWD",     { 1,  1,  "passwd",
                      std::make_shared<Creator<PASSWDCommand>>(),
                      "Syntax: SITE PASSWD <password>",
                      "Change your password" }, },
    { "CHPASS",     { 2,  2,  "chpass|chpassgadmin",
                      std::make_shared<Creator<CHPASSCommand>>(),
                      "Syntax: SITE CHPASS <user> <password>",
                      "Change another user's password" }, },
    { "DELUSER",    { 1,  1,  "deluser|delusergadmin",
                      std::make_shared<Creator<DELUSERCommand>>(),
                      "Syntax: SITE DELUSER <user>",
                      "Delete a user" }, },
    { "DISKFREE",   { 0,  1,  "diskfree",
                      std::make_shared<Creator<DISKFREECommand>>(),
                      "Syntax: SITE DISKFREE [<path>]",
                      "Display disk space free" }, },
    { "READD",      { 1,  1,  "readd|readdgadmin",
                      std::make_shared<Creator<READDCommand>>(),
                      "Syntax: SITE READD <user>",
                      "Readd a deleted user" }, },
    { "PURGE",      { 1,  1,  "purge",
                      std::make_shared<Creator<PURGECommand>>(),
                      "Syntax: SITE PURGE <user>",
                      "Purge a deleted user" }, },
    { "RENUSER",    { 2,  2,  "renuser",
                      std::make_shared<Creator<RENUSERCommand>>(),
                      "Syntax: SITE RENUSER <old user> <new user>",
                      "Rename a user" }, },
    { "ADDUSER",    { 2, -1,  "adduser|addusergadmin",
                      std::make_shared<Creator<ADDUSERCommand>>(),
                      "Syntax: SITE ADDUSER <user> <password> [<ident@ip>..]",
                      "Add new user" }, },
    { "TADDUSER",   { 3, -1,  "tadduser",
                      std::make_shared<Creator<TADDUSERCommand>>(),
                      "Syntax: SITE TADDUSER <template> <user> <password> [<ident@ip>..]",
                      "Add new user based on a template" }, },
    { "ADDIP",      { 2, -1,  "addip|addipown|addipgadmin",
                      std::make_shared<Creator<ADDIPCommand>>(),
                      "Syntax: SITE ADDIP <user> <ident@ip> [<ident@ip>..]",
                      "Add ident@ip to user" }, },
    { "FLAGS",      { 0, 1, "flags|flagsown",
                      std::make_shared<Creator<FLAGSCommand>>(),
                      "Syntax: SITE FLAGS [<user>]",
                      "View your own or another user's flags" }, },
    { "DELIP",      { 2, -1,  "delip|delipown|delipgadmin",
                      std::make_shared<Creator<DELIPCommand>>(),
                      "Syntax: SITE DELIP <user> <ident@ip> [<ident@ip>..]",
                      "Delete ident@ip from user" }, },
    { "CHANGE",     { 3,  -1,  
                      "change|changeflags|changegadmin|changehomedir", 
                      std::make_shared<Creator<CHANGECommand>>(),  
                      CHANGECommand::Syntax(),
                      "Change settings for a user or users" }, },
    { "KICK",       { 1,  1,  "kick",
                      std::make_shared<Creator<KICKCommand>>(),
                      "Syntax: SITE KICK <user>",
                      "Kick a user's connections from the site" }, },
    { "SEEN",       { 1,  1,  "seen",
                      std::make_shared<Creator<SEENCommand>>(),
                      "Syntax: SITE SEEN <user>",
                      "Display last time a user logged in" }, },
    { "USERS",      { 0,  -1, "users",
                      std::make_shared<Creator<USERSCommand>>(),  
                      "Syntax: SITE USERS [<criteria> ..]",
                      "Detailed list of users" }, },
    { "UTIME",      { 2, -1,  "utime",
                      std::make_shared<Creator<UTIMECommand>>(),
                      "Syntax: SITE UTIME <path> <atime-val> <mtime-val> <ctime-val> UTC\n"
                      "        SITE UTIME <mtime-val> <path>",
                      "Set filesystem times" }, },
    { "GIVE",       { 2,  -1,  "give|giveown",
                      std::make_shared<Creator<GIVECommand>>(),
                      "Syntax: SITE GIVE [-S <section>] <user> <credits>G|M [<message>]",
                      "Give credits to user" }, },
    { "TAKE",       { 2, -1,   "take",
                      std::make_shared<Creator<TAKECommand>>(),
                      "Syntax: SITE TAKE [-S <section>] <user> <credits>G|M [<message>]",
                      "Take credits from user" }, },
    { "STATS",      { 0,  1,  "stats|statsown",
                      std::make_shared<Creator<STATSCommand>>(),
                      "Syntax: SITE STATS [<user>]",
                      "Display user's transfer stats" }, },
    { "USER",       { 0,  1,  "user|userown|usergadmin",
                      std::make_shared<Creator<USERCommand>>(),
                      "Syntax: SITE USER [<user>]",
                      "Display user's info" }, },
    { "TAGLINE",    { 1, -1,  "tagline",
                      std::make_shared<Creator<TAGLINECommand>>(),
                      "Syntax: SITE TAGLINE <tagline>",
                      "Change your tagline" }, },
    { "SETPGRP",    { 2,  2,  "setpgrp",
                      std::make_shared<Creator<SETPGRPCommand>>(),
                      "Syntax: SITE SETPGRP <user> <group>",
                      "Set's a user's primary group" }, },
    { "CHGRP",      { 2, -1,  "chgrp",
                      std::make_shared<Creator<CHGRPCommand>>(),
                      "Syntax: SITE CHGRP <user> [+|-|=] <group> [<group> ..]",
                      "Change user's groups" }, },
    { "RANKS",      { 3, -1,  "ranks|ranksalias",
                      std::make_shared<Creator<RANKSCommand>>(),
                      "Syntax: SITE RANKS DAY|WEEK|MONTH|YEAR|ALL UP|DOWN SPEED|KBYTES|FILES [<number>] [<section>] [<acl> ..]",
                      "Display user upload/download rankings" }, },
    { "GPRANKS",    { 3, -1,  "gpranks|gpranksalias",
                      std::make_shared<Creator<GPRANKSCommand>>(),
                      "Syntax: SITE GPRANKS DAY|WEEK|MONTH|YEAR|ALL UP|DOWN SPEED|KBYTES|FILES [<number>] [<section>]",
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
    { "PREDUPE",    { 1,  1,  "predupe",
                      nullptr,
                      "Syntax: SITE PREDUPE <filemask>",
                      "Forcefully dupe all future uploads matching a file mask" }, },
    { "UPDATE",     { 1,  1,  "update",
                      std::make_shared<Creator<UPDATECommand>>(),
                      "Syntax: SITE UPDATE <pathmask>",
                      "Add all directories matching a path mask to the site index" }, },
    { "DUPE",       { 1,  -1, "dupe",
                      std::make_shared<Creator<DUPECommand>>(),
                      "Syntax: SITE DUPE [-MAX <number>] [-FROM <mmddyy>] [-TO <mmddyy>] <string> [<string> .. ]",
                      "Search the dupe database for directories" }, },
    { "NEW",        { 0,  1,  "new",
                      std::make_shared<Creator<NEWCommand>>(),
                      "Syntax: SITE NEW [<number>]",
                      "Display latest directories" }, },
    { "CHOWN",      { 2,  -1,  "chown",
                      std::make_shared<Creator<CHOWNCommand>>(),
                      "Syntax: SITE CHOWN [-R] [user][:[group]] <pathmask>",
                      "Change file and/or directory ownership" }, },
    { "CHMOD",      { 2,  -1,  "chmod",
                      std::make_shared<Creator<CHMODCommand>>(),
                      "Syntax: SITE CHMOD [-R] <symbolic-mode|octal-mode> <pathmask>",
                      "Change file and/or directory permissions" }, },
    { "EMULATE",    { 1,  1,  "emulate",
                      nullptr,
                      "Syntax: SITE EMULATE <user>",
                      "Become another user by temporarily loading their userfile" }, },
    { "TRAFFIC",    { 0,  0,  "traffic",
                      std::make_shared<Creator<TRAFFICCommand>>(),
                      "Syntax: SITE TRAFFIC",
                      "Display traffic statistics" }, },
    { "WHO",        { 0,  0,  "who",
                      std::make_shared<Creator<WHOCommand>>(),
                      "Syntax: SITE WHO",
                      "Display who's online" }, },
    { "SWHO",       { 0,  0,  "swho",
                      std::make_shared<Creator<SWHOCommand>>(),
                      "Syntax: SITE SWHO",
                      "Display detailed who's online" }, },
    { "WIPE",       { 1,  -1,  "wipe",
                      std::make_shared<Creator<WIPECommand>>(),
                      "Syntax: SITE WIPE [-R] <pathmask>",
                      "Wipe a file or directory (and it's contents)" }, },
    { "LOGS",       { 1,  -1, "logs|errorlog|securitylog|eventlog|dblog|siteoplog|debuglog",
                      std::make_shared<Creator<LOGSCommand>>(),
                      "Syntax: SITE -[-MAX <number>] LOGS ERROR|SECURITY|EVENTS|DB|SITEOP|DEBUG [<string> ..]",
                      "Display entries from a system log" }, },
    { "LASTON",     { 0,  -1, "laston",
                      nullptr,
                      "Syntax: SITE LASTON [<number>] [<criteria> ..]",
                      "Display last users online" }, },
    { "CHGADMIN",   { 2,  2,  "chgadmin",
                      std::make_shared<Creator<CHGADMINCommand>>(),
                      "Syntax: SITE CHGADMIN <user> <group",
                      "Toggle gadmin status for a user on the specified group" }, },
    { "GADDUSER",   { 3,  -1, "gadduser|gaddusergadmin",
                      std::make_shared<Creator<GADDUSERCommand>>(),
                      "Syntax: SITE GADDUSER <group> <user> <password> [<ident@ip> ..]",
                      "Add a user directly to a group" }, },
    { "GROUPS",     { 0,  0,  "groups",
                      std::make_shared<Creator<GROUPSCommand>>(),
                      "Syntax: SITE GROUPS",
                      "Display a list of all groups" }, },
    { "GROUP",      { 1,  1,  "group|groupgadmin",
                      std::make_shared<Creator<GROUPCommand>>(),
                      "Syntax: SITE GROUP <group>",
                      "Display detailed group info" }, },
    { "GRPCHANGE",  { 3,  -1,  "grpchange", 
                      std::make_shared<Creator<GRPCHANGECommand>>(),
                      GRPCHANGECommand::Syntax(),
                      "Change settings for a group or groups" }, },
    { "GRPADD",     { 1, -1,  "grpadd",
                      std::make_shared<Creator<GRPADDCommand>>(),
                      "Syntax: SITE GRPADD <group> [<description>]",
                      "Add a group" }, },
    { "GRPDEL",     { 1,  1,  "grpdel",
                      std::make_shared<Creator<GRPDELCommand>>(),
                      "Syntax: SITE GRPDEL <group>",
                      "Delete a group" }, },
    { "GRPREN",     { 2,  2,  "grpren",
                      std::make_shared<Creator<GRPRENCommand>>(),
                      "Syntax: SITE GRPREN <old group> <new group>",
                      "Rename a group" }, },
    { "HELP",       { 0,  1,  "help",
                      std::make_shared<Creator<HELPCommand>>(),
                      "Syntax: SITE HELP [<command>]",
                      "Display site command help" }, },
    { "STAT",       { 0,  0,  "stat",
                      nullptr,
                      "Syntax: SITE STAT",
                      "Display statline" }, },
    { "TIME",       { 0,  0,  "time",
                      std::make_shared<Creator<TIMECommand>>(),
                      "Syntax: SITE TIME",
                      "Display local time" }, },
    { "SEARCH",     { 1,  -1, "search",
                      std::make_shared<Creator<SEARCHCommand>>(),
                      "Syntax: SITE SEARCH [-MAX <number>] <string> [<string> ..]",
                      "Search the site index" }, },
    { "WELCOME",    { 0,  0,  "welcome",
                      std::make_shared<Creator<WELCOMECommand>>(),
                      "Syntax: SITE WELCOME",
                      "Display welcome message" }, },
    { "GOODBYE",    { 0,  0,  "goodbye",
                      std::make_shared<Creator<GOODBYECommand>>(),
                      "Syntax: SITE GOODBYE",
                      "Display goodbye message" }, },
    { "MSG",        { 1,  -1, "msg|msg*|msg=|msg{",
                      std::make_shared<Creator<MSGCommand>>(),
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
                      std::make_shared<Creator<RELOADCommand>>(),
                      "Syntax: SITE RELOAD",
                      "Reload config file" }, },
    { "SHUTDOWN",   { 1,  1,  "shutdownfull|shutdownsiteop",
                      std::make_shared<Creator<SHUTDOWNCommand>>(),
                      "Syntax: SITE SHUTDOWN SITEOP|REOPEN|FULL",
                      "Shutdown to siteop only or full shutdown and exit" }, },
    { "SREPLY",     { 0,  1,  "sreply",
                      std::make_shared<Creator<SREPLYCommand>>(),
                      "Syntax: SITE SREPLY [ON|OFF]",
                      "Turn single line replies on and off" }, }
  };
}

CommandDefOpt Factory::LookupCustom(const std::string& command)
{
  const cfg::SiteCmd* match = nullptr;
  for (auto& siteCmd : cfg::Get().SiteCmd())
  {
    if (siteCmd.Command() == command)
    {
      match = &siteCmd;
      break;
    }
  }
  
  if (!match) return boost::none;

  std::string aclKeyword("custom-" + util::ToLowerCopy(command));
  switch (match->GetType())
  {
    case cfg::SiteCmd::Type::Exec  :
    {
      return boost::make_optional(CommandDef(aclKeyword, std::make_shared<CustomCreator<CustomEXECCommand>>(*match)));
    }
    case cfg::SiteCmd::Type::Text  :
    {
      return boost::make_optional(CommandDef(aclKeyword, std::make_shared<CustomCreator<CustomTEXTCommand>>(*match)));
    }
    case cfg::SiteCmd::Type::Alias :
    {
      return boost::make_optional(CommandDef(aclKeyword, std::make_shared<CustomCreator<CustomALIASCommand>>(*match)));
    }
  }
  
  verify(false);
}

CommandDefOpt Factory::LookupPlugin(ftp::Client& client, const std::string& command)
{
  auto result = client.Plugins().LookupCommand(command);
  if (!result) return boost::none;
  
  std::string aclKeyword("custom-" + util::ToLowerCopy(command));
  plugin::CommandHook& hook = result->first;
  auto creator = std::make_shared<PluginCreator>(*result->second, hook.function);
  return boost::make_optional(CommandDef(aclKeyword, creator));
}

CommandDefOpt Factory::Lookup(ftp::Client& client, const std::string& command, bool noCustom)
{
  CommandDefOpt def;
  if (!noCustom)
  {
    def = LookupCustom(command);
    if (!def) def = LookupPlugin(client, command);
  }
  
  if (!def)
  {
    CommandDefsMap::const_iterator it = factory->defs.find(command);
    if (it != factory->defs.end()) def.reset(it->second);
  }
  return def;
}

std::unordered_set<std::string> Factory::ACLKeywords()
{
  std::unordered_set<std::string> keywords;
  for (auto& kv : Commands())
  {
    std::vector<std::string> curKeywords;
    util::Split(curKeywords, kv.second.ACLKeyword(), "|");
    keywords.insert(curKeywords.begin(), curKeywords.end());
  }
  return keywords;
}

cmd::Command* PluginCreator::Create(ftp::Client& client, const std::string& argStr, const cmd::Args& args)
{
  return new PluginCommand(client, argStr, args, plugin, function);
}


} /* site namespace */
} /* cmd namespace */
