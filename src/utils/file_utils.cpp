//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <stdio.h>
#include <string>
#include <sys/stat.h>

// ----------------------------------------------------------------------------
#if defined(WIN32)
#include <windows.h>
// ----------------------------------------------------------------------------
namespace u8checker
{
    bool hasUnicode(const std::string& u8_path)
    {
        bool has_unicode = false;
        for (char c : u8_path)
        {
            if (static_cast<unsigned char>(c) > 127)
            {
                has_unicode = true;
                break;
            }
        }
        return has_unicode;
    }   // hasUnicode
}   // namespace u8checker

// ----------------------------------------------------------------------------
/** Return a 8.3 filename if u8_path contains unicode character
 */
std::string FileUtils::Private::getShortPath(const std::string& u8_path)
{
    if (!u8checker::hasUnicode(u8_path))
        return u8_path;

    irr::core::stringw w_path = StringUtils::utf8ToWide(u8_path);
    return FileUtils::Private::getShortPathW(w_path);
}   // getShortPath

// ----------------------------------------------------------------------------
std::string FileUtils::Private::getShortPathW(const irr::core::stringw& w_path)
{
    size_t length = GetShortPathNameW(w_path.c_str(), NULL, 0);
    if (length == 0)
    {
        Log::error("FileUtils",
            "Failed to GetShortPathNameW with getting required length.");
        return "";
    }
    std::vector<wchar_t> short_path;
    short_path.resize(length);
    length = GetShortPathNameW(w_path.c_str(), short_path.data(),
        (DWORD)length);
    if (length == 0)
    {
        Log::error("FileUtils",
            "Failed to GetShortPathNameW with writing short path.");
        return "";
    }
    short_path.push_back(0);

    std::string result;
    // Reserve enough space for conversion
    result.resize(length * 4);
    length = WideCharToMultiByte(CP_ACP, 0, short_path.data(), -1, &result[0],
        (int)result.size(), NULL, NULL);
    // Passing -1 as input string length will null terminated the output, so
    // length written included a null terminator
    result.resize(length - 1);
    return result;
}   // getShortPathW

// ----------------------------------------------------------------------------
std::string FileUtils::Private::getShortPathWriting(const std::string& u8_path)
{
    if (!u8checker::hasUnicode(u8_path))
        return u8_path;

    // Create an empty file first if not exist so we can get the short path
    irr::core::stringw w_path = StringUtils::utf8ToWide(u8_path);
    if (_waccess(w_path.c_str(), 0) == -1)
    {
        FILE* fp = _wfopen(w_path.c_str(), L"wb");
        if (!fp)
        {
            Log::error("FileUtils",
                "Failed to create empty file before writing.");
            return "";
        }
        fclose(fp);
    }
    return FileUtils::Private::getShortPathW(w_path);
}   // getShortPathWriting
#endif

// ----------------------------------------------------------------------------
/** fopen() with unicode path capability.
 */
FILE* FileUtils::fopenU8Path(const std::string& u8_path, const char* mode)
{
#if defined(WIN32)
    std::vector<wchar_t> mode_str;
    for (unsigned i = 0; i < strlen(mode); i++)
        mode_str.push_back((wchar_t)mode[i]);
    mode_str.push_back(0);
    return _wfopen(StringUtils::utf8ToWide(u8_path).c_str(), mode_str.data());
#else
    return fopen(u8_path.c_str(), mode);
#endif
}   // fopenU8Path

// ----------------------------------------------------------------------------
/** stat() with unicode path capability.
 */
int FileUtils::statU8Path(const std::string& u8_path, struct stat *buf)
{
#if defined(WIN32)
    struct _stat st;
    int ret = _wstat(StringUtils::utf8ToWide(u8_path).c_str(), &st);
    buf->st_dev = st.st_dev;
    buf->st_ino = st.st_ino;
    buf->st_mode = st.st_mode;
    buf->st_nlink = st.st_nlink;
    buf->st_uid = st.st_uid;
    buf->st_gid = st.st_gid;
    buf->st_rdev = st.st_rdev;
    buf->st_size = (_off_t)st.st_size;
    buf->st_atime = st.st_atime;
    buf->st_mtime = st.st_mtime;
    buf->st_ctime = st.st_ctime;
    return ret;
#else
    return stat(u8_path.c_str(), buf);
#endif
}   // statU8Path

// ----------------------------------------------------------------------------
/** rename() with unicode path capability.
 */
int FileUtils::renameU8Path(const std::string& u8_path_old,
                            const std::string& u8_path_new)
{
#if defined(WIN32)
    return _wrename(StringUtils::utf8ToWide(u8_path_old).c_str(),
        StringUtils::utf8ToWide(u8_path_new).c_str());
#else
    return rename(u8_path_old.c_str(), u8_path_new.c_str());
#endif
}   // renameU8Path
