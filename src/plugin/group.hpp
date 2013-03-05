#ifndef __PLUGIN_GROUP_HPP
#define __PLUGIN_GROUP_HPP

#include "plugin/util.hpp"
#include "acl/group.hpp"

namespace plugin
{

template <typename UnlockingPolicy = NoUnlocking>
class Group : public acl::Group
{
  Group(const acl::Group& group) : acl::Group(group) { }
  Group(acl::Group&& group) : acl::Group(std::move(group)) { }
  
public:
  Group() { throw NotConstructable(); }
  
  util::Error Rename(const std::string& name)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (!acl::Validate(acl::ValidationType::Groupname, name)) return util::Error::Failure("Value error");
    if (!acl::Group::Rename(name)) return util::Error::Failure("Exists");
    return util::Error::Success();
  }

  util::Error SetDescription(const std::string& description)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (!acl::Validate(acl::ValidationType::Tagline, description)) return util::Error::Failure("Value error");
    acl::Group::SetDescription(description);
    return util::Error::Success();
  }
  
  void SetComment(const std::string& comment)
  {
    UnlockingPolicy unlock; (void) unlock;
    acl::Group::SetComment(comment);
  }

  util::Error SetSlots(int slots)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (slots < -1) return util::Error::Failure("Value error");
    acl::Group::SetSlots(slots);
    return util::Error::Success();
  }
  
  util::Error SetLeechSlots(int leechSlots)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (leechSlots < -2) return util::Error::Failure("Value error");
    acl::Group::SetLeechSlots(leechSlots);
    return util::Error::Success();
  }
  
  util::Error SetAllotmentSlots(int allotmentSlots)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (allotmentSlots < -2) return util::Error::Failure("Value error");
    acl::Group::SetAllotmentSlots(allotmentSlots);
    return util::Error::Success();
  }
  
  util::Error SetMaxAllotmentSize(long long maxAllotmentSize)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (maxAllotmentSize < -1) return util::Error::Failure("Value error");
    acl::Group::SetMaxAllotmentSize(maxAllotmentSize);
    return util::Error::Success();
  }
  
  util::Error SetMaxLogins(int maxLogins)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (maxLogins < -1) return util::Error::Failure("Value error");
    acl::Group::SetMaxLogins(maxLogins);
    return util::Error::Success();
  }

  int NumSlotsUsed() const
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::Group::NumSlotsUsed();
  }
  
  int NumMembers() const
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::Group::NumMembers();
  }
  
  int NumLeeches() const
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::Group::NumLeeches();
  }
  
  int NumAllotments() const
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::Group::NumAllotments();
  }
  
  long long TotalAllotmentSize() const
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::Group::TotalAllotmentSize();
  }
  
  void Purge()
  {
    UnlockingPolicy unlock; (void) unlock;
    acl::Group::Purge();
  }
  
  static boost::optional<Group> Load(acl::GroupID gid)
  {
    UnlockingPolicy unlock; (void) unlock;
    auto group = acl::Group::Load(gid);
    if (!group) return boost::none;
    return boost::make_optional(Group(*group));
  }
  
  static boost::optional<Group> Load(const std::string& name)
  {
    return Load(acl::NameToGID(name));
  }
  
  static std::pair<boost::optional<Group>, std::string> 
                Create(const std::string& name, const Group& templateGroup)
  {
    UnlockingPolicy unlock; (void) unlock;
    if (!acl::Validate(acl::ValidationType::Groupname, name)) return std::make_pair(boost::none, "Value Error");
    auto group = acl::Group::FromTemplate(name, templateGroup);
    if (!group) return std::make_pair(boost::none, "Exists");
    return std::make_pair(Group(*group), "Success");
  }
  
  static std::vector<acl::GroupID> GetGIDs(const std::string& multiStr)
  {
    UnlockingPolicy unlock; (void) unlock;
    return acl::Group::GetGIDs(multiStr);
  }
  
  static std::vector<acl::GroupID> GetGIDs() { return GetGIDs("*"); }
  
  static std::vector<Group> GetGroups(const std::string& multiStr)
  {
    UnlockingPolicy unlock; (void) unlock;
    auto groups1 = acl::Group::GetGroups(multiStr);
    std::vector<Group> groups2;
    for (auto& group : groups1)
    {
      groups2.push_back(std::move(group));
    }
    return groups2;
  }
  
  static std::vector<Group> GetGroups() { return GetGroups("*"); }
};

} /* script namespace */

#endif
