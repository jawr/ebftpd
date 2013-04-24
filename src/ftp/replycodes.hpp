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

#ifndef __FTP_REPLYCODES_HPP
#define __FTP_REPLYCODES_HPP

namespace ftp
{

enum ReplyCode
{
  CodeDeferred = -2,
  CodeNotSet = -1,
  NoCode = 0,
  CommandOkay = 200,
  ServiceReady = 220,
  CommandUnrecognised = 500,
  SyntaxError = 501,
  NotImplemented = 502,
  BadCommandSequence = 503,
  ParameterNotImplemented = 504,
  SystemStatus = 211,
  DirectoryStatus = 212,
  FileStatus = 213,
  HelpMessage = 214,
  SystemType = 215,
  ClosingControl = 221,
  ServiceUnavailable = 421,
  DataAlreadyOpen = 125,
  DataOpenNoTransfer = 225,
  CantOpenDataConnection = 425,
  DataClosedOkay = 226,
  DataCloseAborted = 426,
  PassiveMode = 227,
  LongPassiveMode = 228,
  ExtendedPassiveMode = 229,
  UserLoggedIn = 230,
  UserLoggedInSecure = 232,
  SecurityExchangeOkay = 234,
  NotLoggedIn = 530,
  NeedPassword = 331,
  NeedAccount = 332,
  NeedAccountToStor = 532,
  TransferStatusOkay = 150,
  FileActionOkay = 250,
  PathCreated = 257,
  PendingMoreInfo = 350,
  ActionNotOkay = 550,
  ActionAbortedError = 451,
  PageTypeUnknown = 551,
  NoDiskFree = 452,
  BadFilename = 553,
  InvalidRESTParameter = 554,
  ProtocolNotSupported = 522
};

}

#endif
