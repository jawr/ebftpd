#include "cmd/site/factory.hpp"
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
#include "cmd/site/welcome.hpp"
#include "cmd/site/goodbye.hpp"
#include "cmd/site/ranks.hpp"
#include "cmd/site/gpranks.hpp"
#include "cmd/site/diskfree.hpp"
#include "cmd/site/search.hpp"
#include "cmd/site/update.hpp"
#include "cmd/site/new.hpp"
#include "cmd/site/dupe.hpp"
#include "cmd/site/chgadmin.hpp"
#include "cmd/site/tadduser.hpp"

namespace cmd { namespace site
{

std::unique_ptr<Factory> Factory::factory;

Factory::Factory()
{
  defs =
  {
    { "IDLE",       { 0,  1,  "idle",
                      CreatorBasePtr(new Creator<IDLECommand>()),
                      "Syntax: SITE IDLE [<seconds>]",
                      "Changes idle timeout" }, },
    { "VERS",       { 0,  0,  "vers",
                      CreatorBasePtr(new Creator<VERSCommand>()),
                      "Syntax: SITE VERS",
                      "Display server version" }, },
    { "XDUPE",      { 0,  1,  "xdupe",
                      CreatorBasePtr(new Creator<XDUPECommand>()),
                      "Syntax: SITE XDUPE 0|1|2|3|4",
                      "Display / change extended dupe mode" }, },
    { "PASSWD",     { 1,  1,  "passwd",
                      CreatorBasePtr(new Creator<PASSWDCommand>()),
                      "Syntax: SITE PASSWD <password>",
                      "Change your password" }, },
    { "CHPASS",     { 2,  2,  "chpass|chpassgadmin",
                      CreatorBasePtr(new Creator<CHPASSCommand>()),
                      "Syntax: SITE CHPASS <user> <password>",
                      "Change another user's password" }, },
    { "DELUSER",    { 1,  1,  "deluser|delusergadmin",
                      CreatorBasePtr(new Creator<DELUSERCommand>()),
                      "Syntax: SITE DELUSER <user>",
                      "Delete a user" }, },
    { "DISKFREE",   { 0,  1,  "diskfree",
                      CreatorBasePtr(new Creator<DISKFREECommand>()),
                      "Syntax: SITE DISKFREE [<path>]",
                      "Display disk space free" }, },
    { "READD",      { 1,  1,  "readd|readdgadmin",
                      CreatorBasePtr(new Creator<READDCommand>()),
                      "Syntax: SITE READD <user>",
                      "Readd a deleted user" }, },
    { "PURGE",      { 1,  1,  "purge",
                      CreatorBasePtr(new Creator<PURGECommand>()),
                      "Syntax: SITE PURGE <user>",
                      "Purge a deleted user" }, },
    { "RENUSER",    { 2,  2,  "renuser",
                      CreatorBasePtr(new Creator<RENUSERCommand>()),
                      "Syntax: SITE RENUSER <old user> <new user>",
                      "Rename a user" }, },
    { "ADDUSER",    { 2, -1,  "adduser|addusergadmin",
                      CreatorBasePtr(new Creator<ADDUSERCommand>()),
                      "Syntax: SITE ADDUSER <user> <password> [<ident@ip>..]",
                      "Add new user" }, },
    { "TADDUSER",   { 3, -1,  "tadduser",
                      CreatorBasePtr(new Creator<TADDUSERCommand>()),
                      "Syntax: SITE TADDUSER <template> <user> <password> [<ident@ip>..]",
                      "Add new user based on a template" }, },
    { "ADDIP",      { 2, -1,  "addip|addipown|addipgadmin",
                      CreatorBasePtr(new Creator<ADDIPCommand>()),
                      "Syntax: SITE ADDIP <user> <ident@ip> [<ident@ip>..]",
                      "Add ident@ip to user" }, },
    { "FLAGS",      { 0, 1, "flags|flagsown",
                      CreatorBasePtr(new Creator<FLAGSCommand>()),
                      "Syntax: SITE FLAGS [<user>]",
                      "View your own or another user's flags" }, },
    { "DELIP",      { 2, -1,  "delip|delipown|delipgadmin",
                      CreatorBasePtr(new Creator<DELIPCommand>()),
                      "Syntax: SITE DELIP <user> <ident@ip> [<ident@ip>..]",
                      "Delete ident@ip from user" }, },
    { "CHANGE",     { 3,  -1,  
                      "change|changeflags|changegadmin|changehomedir", 
                      CreatorBasePtr(new Creator<CHANGECommand>()),  
                      CHANGECommand::Syntax(),
                      "Change settings for a user or users" }, },
    { "KICK",       { 1,  1,  "kick",
                      CreatorBasePtr(new Creator<KICKCommand>()),
                      "Syntax: SITE KICK <user>",
                      "Kick a user's connections from the site" }, },
    { "SEEN",       { 1,  1,  "seen",
                      CreatorBasePtr(new Creator<SEENCommand>()),
                      "Syntax: SITE SEEN <user>",
                      "Display last time a user logged in" }, },
    { "USERS",      { 0,  -1, "users",
                      CreatorBasePtr(new Creator<USERSCommand>()),  
                      "Syntax: SITE USERS [<criteria> ..]",
                      "Detailed list of users" }, },
    { "GIVE",       { 2,  -1,  "give|giveown",
                      CreatorBasePtr(new Creator<GIVECommand>()),
                      "Syntax: SITE GIVE [-S <section>] <user> <credits>G|M [<message>]",
                      "Give credits to user" }, },
    { "TAKE",       { 2, -1,   "take",
                      CreatorBasePtr(new Creator<TAKECommand>()),
                      "Syntax: SITE TAKE [-S <section>] <user> <credits>G|M [<message>]",
                      "Take credits from user" }, },
    { "STATS",      { 0,  1,  "stats|statsown",
                      CreatorBasePtr(new Creator<STATSCommand>()),
                      "Syntax: SITE STATS [<user>]",
                      "Display user's transfer stats" }, },
    { "USER",       { 0,  1,  "user|userown|usergadmin",
                      CreatorBasePtr(new Creator<USERCommand>()),
                      "Syntax: SITE USER [<user>]",
                      "Display user's info" }, },
    { "TAGLINE",    { 1, -1,  "tagline",
                      CreatorBasePtr(new Creator<TAGLINECommand>()),
                      "Syntax: SITE TAGLINE <tagline>",
                      "Change your tagline" }, },
    { "SETPGRP",    { 2,  2,  "setpgrp",
                      CreatorBasePtr(new Creator<SETPGRPCommand>()),
                      "Syntax: SITE SETPGRP <user> <group>",
                      "Set's a user's primary group" }, },
    { "CHGRP",      { 2, -1,  "chgrp",
                      CreatorBasePtr(new Creator<CHGRPCommand>()),
                      "Syntax: SITE CHGRP <user> [+|-|=] <group> [<group> ..]",
                      "Change user's groups" }, },
    { "RANKS",      { 3, -1,  "ranks|ranksalias",
                      CreatorBasePtr(new Creator<RANKSCommand>()),
                      "Syntax: SITE RANKS DAY|WEEK|MONTH|YEAR|ALL UP|DOWN SPEED|KBYTES|FILES [<number>] [<section>] [<acl> ..]",
                      "Display user upload/download rankings" }, },
    { "GPRANKS",    { 3, -1,  "gpranks|gpranksalias",
                      CreatorBasePtr(new Creator<GPRANKSCommand>()),
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
                      CreatorBasePtr(new Creator<UPDATECommand>()),
                      "Syntax: SITE UPDATE <pathmask>",
                      "Add all directories matching a path mask to the site index" }, },
    { "DUPE",       { 1,  -1, "dupe",
                      CreatorBasePtr(new Creator<DUPECommand>()),
                      "Syntax: SITE DUPE [-MAX <number>] [-FROM <mmddyy>] [-TO <mmddyy>] <string> [<string> .. ]",
                      "Search the dupe database for directories" }, },
    { "NEW",        { 0,  1,  "new",
                      CreatorBasePtr(new Creator<NEWCommand>()),
                      "Syntax: SITE NEW [<number>]",
                      "Display latest directories" }, },
    { "CHOWN",      { 2,  -1,  "chown",
                      CreatorBasePtr(new Creator<CHOWNCommand>()),
                      "Syntax: SITE CHOWN [-R] [user][:[group]] <pathmask>",
                      "Change file and/or directory ownership" }, },
    { "CHMOD",      { 2,  -1,  "chmod",
                      CreatorBasePtr(new Creator<CHMODCommand>()),
                      "Syntax: SITE CHMOD [-R] <symbolic-mode|octal-mode> <pathmask>",
                      "Change file and/or directory permissions" }, },
    { "EMULATE",    { 1,  1,  "emulate",
                      nullptr,
                      "Syntax: SITE EMULATE <user>",
                      "Become another user by temporarily loading their userfile" }, },
    { "TRAFFIC",    { 0,  0,  "traffic",
                      CreatorBasePtr(new Creator<TRAFFICCommand>()),
                      "Syntax: SITE TRAFFIC",
                      "Display traffic statistics" }, },
    { "WHO",        { 0,  0,  "who",
                      CreatorBasePtr(new Creator<WHOCommand>()),
                      "Syntax: SITE WHO",
                      "Display who's online" }, },
    { "SWHO",       { 0,  0,  "swho",
                      CreatorBasePtr(new Creator<SWHOCommand>()),
                      "Syntax: SITE SWHO",
                      "Display detailed who's online" }, },
    { "WIPE",       { 1,  -1,  "wipe",
                      CreatorBasePtr(new Creator<WIPECommand>()),
                      "Syntax: SITE WIPE [-R] <pathmask>",
                      "Wipe a file or directory (and it's contents)" }, },
    { "LOGS",       { 1,  -1, "logs|errorlog|securitylog|eventlog|dblog|siteoplog|debuglog",
                      CreatorBasePtr(new Creator<LOGSCommand>()),
                      "Syntax: SITE -[-MAX <number>] LOGS ERROR|SECURITY|EVENTS|DB|SITEOP|DEBUG [<string> ..]",
                      "Display entries from a system log" }, },
    { "LASTON",     { 0,  -1, "laston",
                      nullptr,
                      "Syntax: SITE LASTON [<number>] [<criteria> ..]",
                      "Display last users online" }, },
    { "CHGADMIN",   { 2,  2,  "chgadmin",
                      CreatorBasePtr(new Creator<CHGADMINCommand>()),
                      "Syntax: SITE CHGADMIN <user> <group",
                      "Toggle gadmin status for a user on the specified group" }, },
    { "GADDUSER",   { 3,  -1, "gadduser|gaddusergadmin",
                      CreatorBasePtr(new Creator<GADDUSERCommand>()),
                      "Syntax: SITE GADDUSER <group> <user> <password> [<ident@ip> ..]",
                      "Add a user directly to a group" }, },
    { "GROUPS",     { 0,  0,  "groups",
                      CreatorBasePtr(new Creator<GROUPSCommand>()),
                      "Syntax: SITE GROUPS",
                      "Display a list of all groups" }, },
    { "GROUP",      { 1,  1,  "group|groupgadmin",
                      CreatorBasePtr(new Creator<GROUPCommand>()),
                      "Syntax: SITE GROUP <group>",
                      "Display detailed group info" }, },
    { "GRPCHANGE",  { 3,  -1,  "grpchange", 
                      CreatorBasePtr(new Creator<GRPCHANGECommand>()),
                      GRPCHANGECommand::Syntax(),
                      "Change settings for a group or groups" }, },
    { "GRPADD",     { 1, -1,  "grpadd",
                      CreatorBasePtr(new Creator<GRPADDCommand>()),
                      "Syntax: SITE GRPADD <group> [<description>]",
                      "Add a group" }, },
    { "GRPDEL",     { 1,  1,  "grpdel",
                      CreatorBasePtr(new Creator<GRPDELCommand>()),
                      "Syntax: SITE GRPDEL <group>",
                      "Delete a group" }, },
    { "GRPREN",     { 2,  2,  "grpren",
                      CreatorBasePtr(new Creator<GRPRENCommand>()),
                      "Syntax: SITE GRPREN <old group> <new group>",
                      "Rename a group" }, },
    { "HELP",       { 0,  1,  "help",
                      CreatorBasePtr(new Creator<HELPCommand>()),
                      "Syntax: SITE HELP [<command>]",
                      "Display site command help" }, },
    { "STAT",       { 0,  0,  "stat",
                      nullptr,
                      "Syntax: SITE STAT",
                      "Display statline" }, },
    { "TIME",       { 0,  0,  "time",
                      CreatorBasePtr(new Creator<TIMECommand>()),
                      "Syntax: SITE TIME",
                      "Display local time" }, },
    { "SEARCH",     { 1,  -1, "search",
                      CreatorBasePtr(new Creator<SEARCHCommand>()),
                      "Syntax: SITE SEARCH [-MAX <number>] <string> [<string> ..]",
                      "Search the site index" }, },
    { "WELCOME",    { 0,  0,  "welcome",
                      CreatorBasePtr(new Creator<WELCOMECommand>()),
                      "Syntax: SITE WELCOME",
                      "Display welcome message" }, },
    { "GOODBYE",    { 0,  0,  "goodbye",
                      CreatorBasePtr(new Creator<GOODBYECommand>()),
                      "Syntax: SITE GOODBYE",
                      "Display goodbye message" }, },
    { "MSG",        { 1,  -1, "msg|msg*|msg=|msg{",
                      CreatorBasePtr(new Creator<MSGCommand>()),
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
                      CreatorBasePtr(new Creator<RELOADCommand>()),
                      "Syntax: SITE RELOAD",
                      "Reload config file" }, },
    { "SHUTDOWN",   { 1,  1,  "shutdownfull|shutdownsiteop",
                      CreatorBasePtr(new Creator<SHUTDOWNCommand>()),
                      "Syntax: SITE SHUTDOWN SITEOP|REOPEN|FULL",
                      "Shutdown to siteop only or full shutdown and exit" }, },
    { "SREPLY",     { 0,  1,  "sreply",
                      CreatorBasePtr(new Creator<SREPLYCommand>()),
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
      def.reset(CommandDef(util::ToLowerCopy(aclKeyword), 
          CreatorBasePtr(new CustomCreator<CustomEXECCommand>(*match))));
      break;
    }
    case cfg::setting::SiteCmd::Type::TEXT  :
    {
      def.reset(CommandDef(util::ToLowerCopy(aclKeyword), 
          CreatorBasePtr(new CustomCreator<CustomTEXTCommand>(*match))));
      break;
    }
    case cfg::setting::SiteCmd::Type::ALIAS :
    {
      def.reset(CommandDef(util::ToLowerCopy(aclKeyword), 
          CreatorBasePtr(new CustomCreator<CustomALIASCommand>(*match))));
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

} /* site namespace */
} /* cmd namespace */
