#ifndef __FTP_STRINGS_HPP
#define __FTP_STRINGS_HPP

#include "util/enum.hpp"

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

template <> const char* util::EnumStrings<String>::values[];

} /* ftp namespace */

#endif
