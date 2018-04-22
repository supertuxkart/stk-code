//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#ifdef __MINGW32__
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include "config/hardware_stats.hpp"

#include "config/user_config.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "online/http_request.hpp"
#include "utils/random_generator.hpp"

#ifdef __APPLE__
#  include <sys/sysctl.h>
#endif

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#ifndef WIN32
#  include <sys/param.h>    // To get BSD macro
#  include <sys/utsname.h>
#endif
#include <vector>


namespace HardwareStats
{

    namespace Private
    {
        /** Stores the OS version, e.g. "Windows 7", or "Fedora 21". */
        static std::string m_os_version;
    }   // namespace Private
    using namespace Private;

// ----------------------------------------------------------------------------
/** Returns the amount of RAM in MB.
 *  (C) 2014-2015 Wildfire Games (0 A.D.), ported by Joerg Henrichs
 */
int getRAM()
{
#ifdef __linux__
    const uint64_t memory_size = (uint64_t)sysconf(_SC_PHYS_PAGES)
                                        * sysconf(_SC_PAGESIZE);
    return int(memory_size / (1024*1024));
#endif

#ifdef WIN32
    MEMORYSTATUSEX mse;
    mse.dwLength = sizeof(mse);
    const bool ok = GlobalMemoryStatusEx(&mse)==TRUE;

    DWORDLONG memory_size = mse.ullTotalPhys;
    // Richter, "Programming Applications for Windows": the reported
    // value doesn't include non-paged pool reserved during boot;
    // it's not considered available to the kernel. (the amount is
    // 528 KiB on a 512 MiB WinXP/Win2k machine). we'll round up
    // to the nearest megabyte to fix this.
    const DWORDLONG mbyte = 1024*1024;
    return (int)ceil(memory_size/mbyte);
#endif

#ifdef __APPLE__
    size_t memory_size = 0;
    size_t len = sizeof(memory_size);
    // Argh, the API doesn't seem to be const-correct
    /*const*/ int mib[2] = { CTL_HW, HW_PHYSMEM };
    sysctl(mib, 2, &memory_size, &len, 0, 0);
    memory_size /= (1024*1024);
    return int(memory_size);
#endif
    Log::error("HW report",
              "No RAM information available for hardware report.");
    return 0;
}   // getRAM

// ----------------------------------------------------------------------------
/** Returns the number of processors on the system.
 *  (C) 2014-2015 Wildfire Games (0 A.D.), ported by Joerg Henrichs
 */
int getNumProcessors()
{
#if defined(__linux__) || defined(__CYGWIN__)
    return sysconf(_SC_NPROCESSORS_CONF);
#endif
#ifdef WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);	// guaranteed to succeed
    return si.dwNumberOfProcessors;
#endif
#ifdef __APPLE__
    // Mac OS X doesn't have sysconf(_SC_NPROCESSORS_CONF)
    int mib[] = { CTL_HW, HW_NCPU };
    int ncpus;
    size_t len = sizeof(ncpus);
    int ret = sysctl(mib, 2, &ncpus, &len, NULL, 0);
    assert(ret != -1);
    return ncpus;
#endif
    Log::error("HW report",
               "Number of processors not available for hardware report.");
    return 0;
}   // getNumProcessors

// ----------------------------------------------------------------------------
/** Tries opening and parsing the specified release file in /etc to find
 *  information about the distro used.
 *  \param filename Full path of the file to open.
 *  \return True if file could be read and valid information was paresed,
 *          false otherwise.
 */
bool readEtcReleaseFile(const std::string &filename)
{
    std::ifstream in(filename);
    std::string s, distro, version;
    while( (distro.empty() || version.empty()) &&
           std::getline(in, s) )
    {
        std::vector<std::string> l = StringUtils::split(s, '=');
        if(l.size()==0) continue;
        if     (l[0]=="NAME"      ) distro  = l[1];
        else if(l[0]=="VERSION_ID") version = l[1];
    }
    if(!distro.empty() && !version.empty())
    {
        distro = StringUtils::replace(distro, "\"", "");
        version = StringUtils::replace(version, "\"", "");
        m_os_version = distro + " " + version;
        return true;
    }
    return false;
}   // readEtcReleaseFile

// ----------------------------------------------------------------------------
/** Identify more details about the OS, e.g. on linux which distro
 *  and which verison; on windows the version number.
 *  \param json Json data structure to store the os info in.
 */
void determineOSVersion()
{
    std::string version, distro;

#ifdef __linux__
    // First try the standard /etc/os-release. Then check for older versions
    // e.g. /etc/fedora-release, /etc/SuSE-release, /etc/redhat-release
    if(readEtcReleaseFile("/etc/os-release")) return;

    std::set<std::string> file_list;
    file_manager->listFiles(file_list, "./", true);
    for(std::set<std::string>::iterator i  = file_list.begin();
                                        i != file_list.end(); i++)
    {
        // Only try reading /etc/*-release files
        if(StringUtils::hasSuffix(*i, "-release"))
            if (readEtcReleaseFile(*i)) return;
    }
    // Fallback in case that we can't find any valid information in /etc/*release
    struct utsname u;
    if (uname(&u))
    {
        m_os_version = "Linux unknown";
        return;
    }
    // Ignore data after "-", since it could identify a system (self compiled
    // kernels).
    std::vector<std::string> l = StringUtils::split(std::string(u.release),'-');
    m_os_version = std::string(u.sysname) + " " + l[0];

#endif

#ifdef BSD
    struct utsname u;
    if (uname(&u))
    {
        m_os_version = "BSD unknown";
        return;
    }
    // Ignore data after "-", since it could identify a system (self compiled
    // kernels).
    std::vector<std::string> l = StringUtils::split(std::string(u.release),'-');
    m_os_version = std::string(u.sysname) + " " + l[0];
#endif

#ifdef WIN32
    //  (C) 2014-2015 Wildfire Games (0 A.D.), ported by Joerg Henrichs.

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
                      KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
    {
        m_os_version = "windows-unknown";
        return;
    }
    char windows_version_string[20];
    DWORD size = sizeof(windows_version_string);
    RegQueryValueEx(hKey, "CurrentVersion", 0, 0, (LPBYTE)windows_version_string, &size);
    unsigned major = 0, minor = 0;

    std::stringstream sstr(windows_version_string);
    sstr >> major;
    if (sstr.peek() == '.')
        sstr.ignore();
    sstr >> minor;

    int windows_version = (major << 8) | minor;
    RegCloseKey(hKey);

    switch(windows_version)
    {
    case 0x0500: m_os_version="Windows 2000";  break;
    case 0x0501: m_os_version="Windows XP";    break;
    case 0x0502: m_os_version="Windows XP64";  break;
    case 0x0600: m_os_version="Windows Vista"; break;
    case 0x0601: m_os_version="Windows 7";     break;
    case 0x0602: m_os_version="Windows 8";     break;
    case 0x0603: m_os_version="Windows 8_1";   break;
    default: {
                 m_os_version = StringUtils::insertValues("Windows %d",
                                                          windows_version);
                 break;
             }
    }   // switch

#endif
}   // determineOSVersion

// ----------------------------------------------------------------------------
/** Returns the OS version, e.g.: "Windows 7", or "Fedora 21".
 */
const std::string& getOSVersion()
{
    if(m_os_version.empty())
        determineOSVersion();
    return m_os_version;
}   // getOSVersion

// ----------------------------------------------------------------------------
/** If the configuration of this installation has not been reported for the
 *  current version, collect the hardware statistics and send it to STK's
 *  server.
 */
void reportHardwareStats()
{
#ifdef SERVER_ONLY
    return;
#else
    if(!UserConfigParams::m_hw_report_enable)
        return;

    // Version of the hw report, which is stored in the DB. If new fields
    // are added, increase this version. Each STK installation will report
    // its configuration only once (per version number). So if the version
    // number is increased, a new report will be sent.
    const int report_version = 1;
    if(UserConfigParams::m_last_hw_report_version>=report_version) return;
    while(UserConfigParams::m_random_identifier==0)
    {
        RandomGenerator rg;
        UserConfigParams::m_random_identifier = rg.get(1<<30);
        user_config->saveConfig();
    }

    Json json;
#ifdef WIN32
    json.add("os_win", 1);
#else
    json.add("os_win", 0);
#endif
#ifdef __APPLE__
    json.add("os_macosx", 1);
#else
    json.add("os_macosx", 0);
#endif
#ifdef ANDROID
    json.add("os_android", 1);
#else
    json.add("os_android", 0);
#endif
#if defined(__linux__) && !defined(ANDROID)
    json.add("os_linux", 1);
    json.add("os_unix", 1);
#else
    json.add("os_linux", 0);
    json.add("os_unix", 0);
#endif
#ifdef DEBUG
    json.add("build_debug", 1);
#endif

    json.add("os_version", getOSVersion());

    unsigned int ogl_version = CVS->getGLSLVersion();
    unsigned int major = ogl_version/100;
    unsigned int minor = ogl_version - 100*major;
    std::string version =
        StringUtils::insertValues("%d.%d", major, minor);
    json.add("GL_SHADING_LANGUAGE_VERSION", version);

    std::string vendor, renderer, full_version;
    irr_driver->getOpenGLData(&vendor, &renderer, &full_version);
    json.add("gfx_drv_ver", "OpenGL "+vendor);

    std::string card_name = vendor;
    if(StringUtils::startsWith(card_name, "ATI Technologies Inc."))
        card_name="ATI";
    else if (StringUtils::startsWith(card_name, "NVIDIA Corporation"))
        card_name="NVIDIA";
    else if(StringUtils::startsWith(card_name, "S3 Graphics"))
        card_name="S3";
    json.add("gfx_card", card_name+" "+renderer);

    json.add("video_xres", UserConfigParams::m_width );
    json.add("video_yres", UserConfigParams::m_height);

    int mem = getRAM();
    if(mem>0)
        json.add("ram_total", mem);

    int nr_procs = getNumProcessors();
    if(nr_procs>0)
        json.add("cpu_numprocs", nr_procs);

#ifndef SERVER_ONLY
    json.add("GL_EXTENSIONS", getGLExtensions());
    getGLLimits(&json);
#endif
    json.finish();

    // ------------------------------------------------------------------------
    /** A small class which sends the HW report to the STK server. On
     *  completion, it will either update the last-submitted-hw-report version,
     *  or log an error message (in which case next time STK is started it
     *  wil try again to log the report).
     */
    class HWReportRequest : public Online::HTTPRequest
    {
    private:
        /** Version number of the hw report. */
        int m_version;
    public:
        HWReportRequest(int version) : Online::HTTPRequest(/*manage memory*/true, 1)
                                     , m_version(version)
        {}
        // --------------------------------------------------------------------
        /** Callback after the request has been executed.
         */
        virtual void callback()
        {
            // If the request contains incorrect data, it will not have a
            // download error, but return an error string as return value:
            if(hadDownloadError() || getData()=="<h1>Bad Request (400)</h1>")
            {
                Log::error("HW report", "Error uploading the HW report.");
                if(hadDownloadError())
                    Log::error("HW report", "%s", getDownloadErrorMessage());
                else
                    Log::error("HW report", "%s", getData().c_str());
            }
            else
            {
                Log::info("HW report", "Upload successful.");
                UserConfigParams::m_last_hw_report_version = m_version;
                // The callback is executed by the main thread, so no need
                // to worry about locks when writing the file.
                user_config->saveConfig();
            }
        }   // callback

    };   // HWReportRequest
    // ------------------------------------------------------------------------

    Online::HTTPRequest *request = new HWReportRequest(report_version);
    request->addParameter("user_id", UserConfigParams::m_random_identifier);
    request->addParameter("time", StkTime::getTimeSinceEpoch());
    request->addParameter("type", "hwdetect");
    request->addParameter("version", report_version);
    request->addParameter("data", json.toString());
    request->setURL((std::string)UserConfigParams::m_server_hw_report+"/upload/v1/");
    //request->setURL("http://127.0.0.1:8000/upload/v1/");
    request->queue();
#endif   // !SERVER_ONLY
}   // reportHardwareStats

}   // namespace HardwareStats
