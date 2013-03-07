#ifndef __PLUGIN_HOOKS_HPP
#define __PLUGIN_HOOKS_HPP

#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <unordered_map>
#include <boost/variant.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/uuid/uuid.hpp>
#include "util/verify.hpp"
#include "util/string.hpp"
#include "acl/acl.hpp"
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

class Client;

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

struct CommandHook
{
  CommandHookFunction function;
  HookID id;
  std::string command;
  std::string description;
  std::string syntax;
  acl::ACL acl;
  
  CommandHook(const CommandHookFunction& function, HookID&& id,
              const std::string& command, 
              const std::string& description,
              const std::string& syntax,
              const std::string& acl) :
    function(function),
    id(id),
    command(command),
    description(description),
    syntax(syntax),
    acl(acl)
  { }
};


class CommandHooks
{
private:
  std::unordered_map<std::string, CommandHook> hooks;
  
  static const CommandHook& TakeSecond(const std::pair<std::string, CommandHook>& kv)
  {
    return kv.second;
  }
  
public:
  typedef decltype(boost::make_transform_iterator(hooks.cbegin(), TakeSecond)) iterator;

  boost::optional<CommandHook> Lookup(const std::string& command);
  boost::optional<HookID> Connect(const std::string& command, const std::string& description, 
                                  const std::string& syntax, const std::string& acl, 
                                  const CommandHookFunction& function);
  void Disconnect(const HookID& id);
  
  iterator begin()
  {
    return boost::make_transform_iterator(hooks.begin(), TakeSecond);
  }
  
  iterator end()
  {
    return boost::make_transform_iterator(hooks.end(), TakeSecond);
  }
};

class EventHooks
{
  struct EventHook
  {
    EventHookFunction function;
    HookID id;
    bool always;
    
    EventHook(const EventHookFunction& function, HookID&& id, bool always) :
      function(function),
      id(id),
      always(always)
    { }
  };
  
  std::unordered_multimap<Event, EventHook> hooks;

public:
  void Trigger(Event event, Client& client, const EventHookArgs& args, bool& okay);
  HookID Connect(Event event, const EventHookFunction& function, bool always);
  void Disconnect(const HookID& id);
};

std::string HookIDToString(const HookID& id);
bool HookIDFromString(const std::string& s, HookID& id);
bool IntegerToHookResult(int iresult, HookResult& result);

} /* plugin namespace */

#endif
