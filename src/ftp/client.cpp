#include "ftp/client.hpp"
#include "ftp/clientimpl.hpp"

namespace ftp
{

Client::Client() :
  pimpl(new ClientImpl(*this))
{
}

Client::~Client()
{
}

acl::User& Client::User()
{
  return pimpl->User();
}

const acl::User& Client::User() const
{
  return pimpl->User();
}

bool Client::Accept(util::net::TCPListener& server)
{
  return pimpl->Accept(server);
}

/*bool Client::IsFinished() const
{
  return pimpl->IsFinished();
}*/

void Client::SetLoggedIn(bool kicked)
{
  pimpl->SetLoggedIn(kicked);
}

void Client::SetWaitingPassword(const acl::User& user, bool kickLogin)
{
  pimpl->SetWaitingPassword(user, kickLogin);
}

bool Client::VerifyPassword(const std::string& password)
{
  return pimpl->VerifyPassword(password);
}

bool Client::PasswordAttemptsExceeded() const
{
  return pimpl->PasswordAttemptsExceeded();
}

void Client::SetRenameFrom(const fs::VirtualPath& path)
{
  pimpl->SetRenameFrom(path);
}

const fs::VirtualPath& Client::RenameFrom() const
{
  return pimpl->RenameFrom();
}

bool Client::KickLogin() const
{
  return pimpl->KickLogin();
}

::ftp::Control& Client::Control() 
{
  return pimpl->Control();
}

const ::ftp::Control& Client::Control() const
{
  return pimpl->Control();
}

::ftp::Data& Client::Data()
{
  return pimpl->Data();
}

const ::ftp::Data& Client::Data() const
{
  return pimpl->Data();
}

util::ProcessReader& Client::Child()
{
  return pimpl->Child();
}

void Client::SetIdleTimeout(const boost::posix_time::seconds& idleTimeout)
{
  pimpl->SetIdleTimeout(idleTimeout);
}

const boost::posix_time::seconds& Client::IdleTimeout() const
{
  return pimpl->IdleTimeout();
}

const boost::posix_time::ptime& Client::LoggedInAt() const
{
  return pimpl->LoggedInAt();
}

void Client::SetXDupeMode(xdupe::Mode xdupeMode)
{
  pimpl->SetXDupeMode(xdupeMode);
}

xdupe::Mode Client::XDupeMode() const
{
  return pimpl->XDupeMode();
}

/*bool Client::IsFxp(const util::net::Endpoint& ep) const
{
  return pimpl->IsFxp(ep);
}*/

bool Client::ConfirmCommand(const std::string& argStr)
{
  return pimpl->ConfirmCommand(argStr);
}

boost::posix_time::seconds Client::IdleTime() const 
{ 
  return pimpl->IdleTime();
}

const std::string& Client::CurrentCommand() const
{
  return pimpl->CurrentCommand();
}

ClientState Client::State() const
{
  return pimpl->State();
}

void Client::SetState(ClientState state)
{
  pimpl->SetState(state);
}

plugin::PluginCollection& Client::Plugins()
{
  return pimpl->Plugins();
}

const plugin::PluginCollection& Client::Plugins() const
{
  return pimpl->Plugins();
}

void Client::Interrupt()
{
  pimpl->Interrupt();
}

void Client::LogTraffic() const
{
  pimpl->LogTraffic();
}

bool Client::SetSiteopOnly()
{
  return ClientImpl::SetSiteopOnly();
}

bool Client::SetReopen()
{
  return ClientImpl::SetReopen();
}

bool Client::IsSiteopOnly()
{
  return ClientImpl::IsSiteopOnly();
}

bool Client::PostCheckAddress()
{
  return pimpl->PostCheckAddress();
}

bool Client::PreCheckAddress()
{
  return pimpl->PreCheckAddress();
}

std::string Client::IP(LogAddresses log) const
{
  return pimpl->IP(log);
}

std::string Client::Ident(LogAddresses log) const
{
  return pimpl->Ident(log);
}

std::string Client::Hostname(LogAddresses log) const
{
  return pimpl->Hostname(log);
}

void Client::HostnameLookup()
{
  pimpl->HostnameLookup();
}

std::string Client::HostnameAndIP(LogAddresses log) const
{
  return pimpl->HostnameAndIP(log);
}

bool Client::IdntUpdate(const std::string& ident, std::string ip,
                        const std::string& hostname)
{
  return pimpl->IdntUpdate(ident, ip, hostname);
}

bool Client::IdntParse(const std::string& command)
{
  return pimpl->IdntParse(command);
}

void Client::SetUserUpdated()
{
  pimpl->SetUserUpdated();
}

void Client::Start()
{
  pimpl->Start();
}

void Client::Join()
{
  pimpl->Join();
}

bool Client::TryJoin()
{
  return pimpl->TryJoin();
}

} /* ftp namespace */
