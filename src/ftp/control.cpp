//    Copyright (C) 2012, 2013 ebftpd team
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ftp/control.hpp"
#include "ftp/controlimpl.hpp"

namespace ftp
{

Control::Control() :
  pimpl(new ControlImpl(&socket)),
  PartFormat(boost::bind(&ControlImpl::PartReply, &*pimpl, _1, _2)),
  Format(boost::bind(&ControlImpl::Reply, &*pimpl, _1, _2))
{
}

Control::~Control()
{
}

void Control::Accept(util::net::TCPListener& listener)
{
  pimpl->Accept(listener);
}

std::string Control::NextCommand(const boost::posix_time::time_duration* timeout)
{
  return pimpl->NextCommand(timeout);
}

void Control::PartReply(ReplyCode code, const std::string& message)
{
  pimpl->PartReply(code, message);
}

void Control::Reply(ReplyCode code, const std::string& message)
{
  pimpl->Reply(code, message);
}

void Control::SetSingleLineReplies(bool singleLineReplies)
{
  pimpl->SetSingleLineReplies(singleLineReplies);
}

bool Control::SingleLineReplies() const
{
  return pimpl->SingleLineReplies();
}

void Control::NegotiateTLS()
{
  pimpl->NegotiateTLS();
}

void Control::Write(const char* buffer, size_t len)
{
  pimpl->Write(buffer, len);
}

const util::net::Endpoint& Control::RemoteEndpoint() const
{
  return pimpl->RemoteEndpoint();
}

const util::net::Endpoint& Control::LocalEndpoint() const
{
  return pimpl->LocalEndpoint();
}

bool Control::IsTLS() const
{
  return pimpl->IsTLS();
}

std::string Control::TLSCipher() const
{
  return pimpl->TLSCipher();
}

void Control::Interrupt()
{
  pimpl->Interrupt();
}

long long Control::BytesRead() const
{
  return pimpl->BytesRead();
}

long long Control::BytesWrite() const
{
  return pimpl->BytesWrite();
}

std::string Control::WaitForIdnt()
{
  return pimpl->WaitForIdnt();
}

} /* ftp namespace */
