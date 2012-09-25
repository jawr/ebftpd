#ifndef __FTP_REPLYCODES_HPP
#define __FTP_REPLYCODES_HPP

namespace ftp
{

enum ReplyCode
{
  CodeNotSet = -1,
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
  ServiceUnavailable = 412,
  DataAlreadyOpen = 125,
  DataOpenNoTransfer = 225,
  CantOpenDataConnection = 425,
  DataClosedOkay = 226,
  DataCloseAborted = 426,
  PassiveMode = 227,
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
  BadFilename = 553
};

}

#endif
