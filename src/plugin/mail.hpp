#ifndef __PLUGIN_MAIL_HPP
#define __PLUGIN_MAIL_HPP

#include "db/mail/message.hpp"

namespace plugin
{

struct MailStatus
{
	std::string Unread() const { return util::EnumToString(db::mail::Status::Unread); }
  std::string Trash() const { return util::EnumToString(db::mail::Status::Trash); }
  std::string Saved() const { return util::EnumToString(db::mail::Status::Saved); }
};

} /* script namespace */

#endif
