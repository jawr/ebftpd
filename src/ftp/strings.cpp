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

#include "ftp/strings.hpp"

namespace util
{

template <> const char* util::EnumStrings<ftp::String>::values[] = 
{
  "%1% command successful",
  "TLS not enabled",
  "AUTH %1% not supported",
  "AUTH %1% successful",
  "%1%: %2%",
  "Unable to listen for data connection: %1%",
  "Entering passive mode (%1%)",
  "%1%",
  "CWD command successful (Alias: %1%)",
  "CWD command successful (Matched: %1%)",
  "CWD command successful (Cdpath: %1%)",
  "DELE command succesful (%1$.2f MB credits lost)",
  "Unable to open data connection: %1%",
  "EPSV %1% not supported",
  "%1% (%2%)",
  "Entering extended passive mode (%1%)",
  "Opening connection for directory listing ..",
  "Opening connection for directory listing using TLS/SSL ..",
  "TLS is enforced on directory listing",
  "Error while writing to data connection: %1%",
  "Error while reading from data connection: %1%",
  "End of directory listing (%1%)",
  "Path name contains one or more invalid characters",
  "Transfer mode '%1%' not implemented",
  "Transfer mode set to '%1%'",
  "Unrecognised transfer mode",
  "Password attempts exceeded, disconnecting",
  "Login incorrect",
  "Login not allowed from %1%",
  "You have been deleted. Bye bye",
  "Server is closed for maintenance",
  "Unable to change to site root directory: %1%",
  "Unable to load user profile",
  "Your account has expired"
  "Kicked %1% (idle %2%) of %1% active login(s)",
  "User %1% logged in",
  "Only protection buffer size 0 supported",
  "Protection buffer size set to 0",
  "Protection type '%1%' not implemented",
  "Unrecognised protection type",
  "Protection type set to '%1%'",
  "\"%1%\" is your working directory",
  "Bye bye",
  "Invalid parameter, restart offset set to 0",
  "Restart offset set to %1%",
  "Restart offset larger than file size",
  "Resume not supported for ASCII data type",
  "You have reached your maximum of %1% simultaneous download(s)",
  "You have reached your maximum of %1% simultaneous upload(s)",
  "File can't be downloaded in ASCII, change to BINARY",
  "Opening %1% connection for download of %1% (%1% bytes) ..",
  "Opening %1% connection for download of %1% (%1% bytes) using TLS/SSL ..",
  "Opening %1% connection for upload of %1% ..",
  "Opening %1% connection for upload of %1% using TLS/SSL ..",
  "TLS is enforced on data transfers",
  "TLS is enforced on FXP transfers",
  "Error while reading from disk: %1%",
  "Error while writing to disk: %1%",
  "File exists, ready for destination name",
  "%1% command not understood",
  "%1% permission denied",
  "%1% not implemented",
  "SSCN:%1% METHOD",
  "Status of %1%:",
  "End of status (%1%)",
  "File is being uploaded by %1%",
  "File already uploaded by %1% (%2% ago)",
  "File is being uploaded",
  "File already uploaded (%1% arg)",
  "Failed to chmod uploaded file: %1%",
  "Unable to generate unique filename",
  "UNIX Type: L8",
  "TYPE %1% not supported",
  "TLS enforced on control connection",
  "User %1% access denied",
  "Password required for %1%",
  ""
};

} /* util namespace */
