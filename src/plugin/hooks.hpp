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
#include <boost/iterator/transform_iterator.hpp>
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

inline HookID GenerateID()
{
  boost::mt19937 ran(time(NULL));
  boost::uuids::random_generator generateID(&ran);
  return generateID();
}

enum class Event
{
  Connected,
  Disconnected,
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

class CommandHooks
{
public:
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
  
private:
  int pluginCount;    // plugin count kept to ensure hooks are not cleared until all plugins destroyed
  std::unordered_map<std::string, CommandHook> commandHooks;
  
  static boost::thread_specific_ptr<CommandHooks> instance;

  static const CommandHook& TakeSecond(const std::pair<std::string, CommandHook>& kv)
  {
    return kv.second;
  }
  
public:
  CommandHooks() : pluginCount(pluginCount) { }
  
  void Acquire()
  {
    ++pluginCount;
  }
  
  void Release()
  {
    if (--pluginCount <= 0)
    {
      verify(pluginCount == 0);
      commandHooks.clear();
    }
  }

  typedef decltype(boost::make_transform(commandHooks.cbegin(), TakeSecond) iterator;

  bool Trigger(ftp::Client& client, const std::string& commandLine, const std::vector<std::string>& args)
  {
    assert(args.size() >= 2);
    assert(args[0] == "SITE");
    auto it = commandHooks.find(args[1]);
    if (it == commandHooks.end()) return false;
    if (!it->second.acl.Evaluate(client.User().ACLInfo())) throw cmd::PermissionError();
    Client pluginClient(client);
    HookResult result = it->second.function(pluginClient, commandLine, args);
    // check result -- possibly do some logging
    return true;
  }
  
  boost::optional<HookID> Connect(const std::string& command, const std::string& description, 
                                  const std::string& acl, const CommandHookFunction& function)
  {
    // site factory singleton needs to be fixed
    //if (cmd::site::Factory::Lookup(command)) return boost::none;
    auto commandHook = CommandHook(function, GenerateID(),command, description, acl);
    auto it = commandHooks.insert(std::make_pair(command, std::move(commandHook)));
    if (!it.second) return boost::none;
    return boost::make_optional(it.first->second.id);
  }
  
  void Disconnect(const HookID& id)
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
  
  static EventHooks& Get()
  {
    Hooks* hooks = instance.get();
    if (!hooks)
    {
      hooks = new Hooks();
      instance.reset(hooks);
    }
    return *hooks;
  }
  
  iterator begin()
  {
    return boost::make_transform_iterator(commandHooks.begin(), TakeSecond);
  }
  
  iterator end()
  {
    return boost::make_transform_iterator(commandHooks.end(), TakeSecond);
  }
  
};

class EventHooks
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
  
  std::unordered_multimap<Event, EventHook> eventHooks;
    
  static boost::thread_specific_ptr<Hooks> instance;

public:
  ~Hooks()
  {
    // must always be cleared before lua_State is closed
    // so must be cleared explicitly using Clear()
    verify(eventHooks.empty());
  }

  void Trigger(Event event, ftp::Client& client, const EventHookArgs& args)
  {
    auto range = eventHooks.equal_range(event);
    for (auto it = range.first; it != range.second; ++it)
    {
      Client pluginClient(client);
      it->second.function(pluginClient, args);
    }
  }

  void Trigger(Event event, ftp::Client& client)
  {
    Trigger(event, client, EventHookArgs());
  }
  
  HookID Connect(Event event, const EventHookFunction& function)
  {
    auto it = eventHooks.insert(std::make_pair(event, EventHook(function, GenerateID())));
    return it->second.id;
  }
  
  void Disconnect(const HookID& id)
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
  
  void Clear()
  {
    eventHooks.clear();
  }
  
  static EventHooks& Get()
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

/* script namespace */

#endif
