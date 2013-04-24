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

#ifndef __FTP_STRINGS_HPP
#define __FTP_STRINGS_HPP

#include "util/enumstrings.hpp"

namespace ftp
{

enum class String
{
  CommandSuccess,
  TLSNotEnabled,
  AuthTypeNotSupported,
  AuthSuccess,
  FileActionError,
  FailDataListen,
  PassiveMode,
  GenericError,
  CWDAlias,
  CWDMatched,
  CWDCdpath,
  DeleteWithCreditLoss,
  FailDataOpen,
  EPSVModeNotSupported,
  EPSVModeStatus,
  ExtendedPassiveMode,
  OpenDirectoryList,
  OpenDirectoryListTLS,
  TLSEnforcedDirectoryList,
  DataWriteError,
  DataReadError,
  CloseDirectoryList,
  InvalidPathName,
  ModeNotImplemented,
  ModeSuccess,
  ModeUnrecognised,
  PasswordAttempsExceeded,
  LoginIncorrect,
  BadIdentAddress,
  LoginDeleted,
  ServerShutdown,
  SiteRootUnavailable,
  AccountExpired,
  LoginKick,
  LoggedIn,
  ProtectionSizeInvalid,
  ProtectionSizeSuccess,
  ProtectionTypeNotImplemented,
  ProtectionTypeUnrecognised,
  ProtectionSuccess,
  WorkingDirectory,
  CloseControl,
  InvalidRestartOffset,
  RestartSuccess,
  RestartTooLarge,
  ASCIIResumeNotSupported,
  MaximumDownloads,
  MaximumUploads,
  InvalidASCIIFileType,
  OpenDownload,
  OpenDownloadTLS,
  OpenUpload,
  OpenUploadTLS,
  TLSEnforcedData,
  TLSEnforcedFXP,
  DiskReadError,
  DiskWriteError,
  RenameFromSuccess,
  CommandNotUnderstood,
  CommandPermissionDenied,
  SSCNMethod,
  StartStatusDirectoryList,
  EndStatusDirectoryList,
  DupeUploadInProgress,
  DupeUploadComplete,
  DupeUploadInProgressHideOwner,
  DupeUploadCompleteHideOwner,
  UploadChmodFail,
  UniqueFilenameFail,
  SYSTResponse,
  DataTypeNotSupported,
  TLSEnforcedControl,
  LoginAcessDenied,
  PasswordRequired
};

} /* ftp namespace */

namespace util
{
template <> const char* util::EnumStrings<ftp::String>::values[];
}

#endif
