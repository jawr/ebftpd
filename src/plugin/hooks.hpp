#ifndef __PLUGIN_HOOKS_HPP
#define __PLUGIN_HOOKS_HPP

#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <boost/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/thread/tss.hpp>
#include "util/verify.hpp"
#include "util/string.hpp"
#include "acl/acl.hpp"
#include "cmd/site/factory.hpp"
#include "cmd/error.hpp"
#include "plugin/client.hpp"

namespace plugin
{
enum class Event;
}

namespace std
{
template <>
class hash<plugin::Event> : public unary_function<plugin::Event, size_t>
{
  std::hash<int> h;
  
public:
  size_t operator()(const plugin::Event& value) const
  {
    return h(static_cast<int>(value));
  }
};
} 

namespace plugin
{

enum class Event
{
  Load,
  Unload,
  LoggedIn,
  LoggedOut,
  BeforeCommand,
  AfterCommandOkay,
  AfterCommandFail,
  BeforeUpload,
  AfterUpload,
  UploadOkay,
  UploadFail,
  BeforeDownload,
  AfterDownload,
  DownloadOkay,
  DownloadFail
};

enum class HookResult
{
  Okay,
  Error,
  Failure,
  NoReturnValue,
  InvalidReturnValue,
  
  // values only for bounds checking
  First = Okay,
  Last = InvalidReturnValue
};

typedef boost::variant<std::string, long long, double, int> EventHookArg;
typedef std::vector<EventHookArg> EventHookArgs;
typedef std::function<HookResult(Client&, const EventHookArgs&)> EventHookFunction;
typedef std::function<HookResult(Client&, const std::string&, const std::vector<std::string>&)> CommandHookFunction;
typedef boost::uuids::uuid HookID;

class Hooks
{
  struct EventHook
  {
    EventHookFunction function;
    HookID id;
    
    EventHook(const EventHookFunction& function, HookID&& id) :
      function(function),
      id(id)
    { }
  };
  
  struct CommandHook
  {
    CommandHookFunction function;
    HookID id;
    std::string command;
    std::string description;
    acl::ACL acl;
    
    CommandHook(const CommandHookFunction& function, HookID&& id,
                const std::string& command, 
                const std::string& description,
                const std::string& acl) :
      function(function),
      id(id),
      command(command),
      description(description),
      acl(acl)
    { }
  };
  
  std::unordered_multimap<Event, EventHook> eventHooks;
  std::unordered_map<std::string, CommandHook> commandHooks;
  
  boost::mt19937 ran;
  boost::uuids::random_generator generateID;
  
  static boost::thread_specific_ptr<Hooks> instance;

  Hooks() : 
    ran(time(NULL)),
    generateID(&ran)
  { }

public:
  ~Hooks()
  {
    // must always be cleared before lua_State is closed
    // so must be cleared explicitly using Clear()
    verify(eventHooks.empty());
  }

  void TriggerEvent(Event event, Client& client, const EventHookArgs& args)
  {
    auto range = eventHooks.equal_range(event);
    for (auto it = range.first; it != range.second; ++it)
    {
      it->second.function(client, args);
    }
  }
  
  HookID ConnectEvent(Event event, const EventHookFunction& function)
  {
    auto it = eventHooks.insert(std::make_pair(event, EventHook(function, generateID())));
    return it->second.id;
  }
  
  void DisconnectEvent(const HookID& id)
  {
    auto it = std::find_if(eventHooks.begin(), eventHooks.end(),
                  [&id](const std::pair<Event, EventHook>& kv)
                  {
                    return kv.second.id == id;
                  });
    if (it != eventHooks.end())
    {
      eventHooks.erase(it);
    }
  }
  
  bool TriggerCommand(Client& client, const std::string& commandLine, const std::vector<std::string>& args)
  {
    assert(args.size() >= 2);
    assert(args[0] == "SITE");
    auto it = commandHooks.find(args[1]);
    if (it == commandHooks.end()) return false;
    if (!it->second.acl.Evaluate(client.User())) throw cmd::PermissionError();
    HookResult result = it->second.function(client, commandLine, args);
    // check result -- possibly do some logging
    return true;
  }
  
  boost::optional<HookID> ConnectCommand(std::string command, const std::string& description, 
                                         const std::string& acl, const CommandHookFunction& function)
  {
    util::ToUpper(command);
    // site factory singleton needs to be fixed
    //if (cmd::site::Factory::Lookup(command)) return boost::none;
    auto it = commandHooks.insert(std::make_pair(command, 
                  CommandHook(function, generateID(),command, 
                              description, acl)));
    if (!it.second) return boost::none;
    return boost::make_optional(it.first->second.id);
  }
  
  void DisconnectCommand(const HookID& id)
  {
    auto it = std::find_if(commandHooks.begin(), commandHooks.end(),
                  [&id](const std::pair<std::string, CommandHook>& kv)
                  {
                    return kv.second.id == id;
                  });
    if (it != commandHooks.end())
    {
      commandHooks.erase(it);
    }
  }
  
  void Clear()
  {
    eventHooks.clear();
    commandHooks.clear();
  }
  
  static Hooks& Get()
  {
    Hooks* hooks = instance.get();
    if (!hooks)
    {
      hooks = new Hooks();
      instance.reset(hooks);
    }
    return *hooks;
  }
};

inline std::string HookIDToString(const HookID& id)
{
  return boost::uuids::to_string(id);
}

inline bool HookIDFromString(const std::string& s, HookID& id)
{
  std::istringstream is(s);
  is >> id;
  return !is.bad() && !is.fail();
}

inline bool IntegerToHookResult(int iresult, HookResult& result)
{
  if (iresult < static_cast<int>(HookResult::First) ||
      iresult > static_cast<int>(HookResult::Last))
  {
    return false;
  }
  
  result = static_cast<HookResult>(iresult);
  return true;
}

} /* script namespace */

#endif
