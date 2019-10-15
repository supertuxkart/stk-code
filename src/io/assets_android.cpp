//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#include "graphics/irr_driver.hpp"
#include "io/assets_android.hpp"
#include "io/file_manager.hpp"
#include "utils/log.hpp"
#include "utils/progress_bar_android.hpp"

#include <cassert>
#include <cstdlib>
#include <fstream>

#ifdef ANDROID
#include <android/asset_manager.h>
#include <sys/statfs.h> 

#include "../../../lib/irrlicht/source/Irrlicht/CIrrDeviceAndroid.h"
#endif

//-----------------------------------------------------------------------------
/** Assets Android constructor
 */
AssetsAndroid::AssetsAndroid(FileManager* file_manager)
{
    m_file_manager = file_manager;
    m_progress_bar = NULL;
}

//-----------------------------------------------------------------------------
/** A function that detects a path where data directory is placed and that
 *  sets some environment variables that are used for finding data and
 *  home directory in a common code.
 */
void AssetsAndroid::init()
{
#ifdef ANDROID
    if (m_file_manager == NULL)
        return;
        
    if (!global_android_app)
        return;

    bool needs_extract_data = false;
    const std::string version = std::string("supertuxkart.") + STK_VERSION;

    // Add some paths to check
    std::vector<std::string> paths;

    if (getenv("SUPERTUXKART_DATADIR"))
        paths.push_back(getenv("SUPERTUXKART_DATADIR"));
        
    if (global_android_app->activity->externalDataPath)
    {
        m_file_manager->checkAndCreateDirectoryP(
                                global_android_app->activity->externalDataPath);
        paths.push_back(global_android_app->activity->externalDataPath);
    }

    if (global_android_app->activity->internalDataPath)
    {
        m_file_manager->checkAndCreateDirectoryP(
                                global_android_app->activity->internalDataPath);
        paths.push_back(global_android_app->activity->internalDataPath);
    }

    if (getenv("EXTERNAL_STORAGE"))
        paths.push_back(getenv("EXTERNAL_STORAGE"));

    if (getenv("SECONDARY_STORAGE"))
        paths.push_back(getenv("SECONDARY_STORAGE"));
        
    paths.push_back("/sdcard/");
    paths.push_back("/storage/sdcard0/");
    paths.push_back("/storage/sdcard1/");
    
#if !defined(ANDROID_PACKAGE_NAME) || !defined(ANDROID_APP_DIR_NAME)
    #error
#endif

    std::string package_name = ANDROID_PACKAGE_NAME;
    paths.push_back("/data/data/" + package_name + "/files/");

    std::string app_dir_name = ANDROID_APP_DIR_NAME;

    // Check if STK data for current version is available somewhere
    for (std::string path : paths)
    {
        Log::info("AssetsAndroid", "Check data files in: %s", path.c_str());
        
        if (!isWritable(path))
        {
            Log::info("AssetsAndroid", "Path doesn't have write access.");
            continue;
        }

        if (m_file_manager->fileExists(path + "/" + app_dir_name + "/data/" + version))
        {
            m_stk_dir = path + "/" + app_dir_name;
            break;
        }
        
        // Stk is an alias of supertuxkart for compatibility with older version.
        if (app_dir_name == "supertuxkart" && 
            m_file_manager->fileExists(path + "/stk/data/" + version))
        {
            m_stk_dir = path + "/stk";
            break;
        }
    }
    
    // If data for current version is not available, then try to find any other
    // version, so that we won't accidentaly create second STK directory in
    // different place
    if (m_stk_dir.size() == 0)
    {
        for (std::string path : paths)
        {
            Log::info("AssetsAndroid", "Check data files for different STK "
                                       "version in: %s", path.c_str());
                                       
            if (!isWritable(path))
            {
                Log::info("AssetsAndroid", "Path doesn't have write access.");
                continue;
            }
    
            if (m_file_manager->fileExists(path + "/" + app_dir_name + "/.extracted"))
            {
                m_stk_dir = path + "/" + app_dir_name;
                needs_extract_data = true;
                break;
            }
    
            // Stk is an alias of supertuxkart for compatibility with older version.
            if (app_dir_name == "supertuxkart" && 
                m_file_manager->fileExists(path + "/stk/.extracted"))
            {
                m_stk_dir = path + "/stk";
                needs_extract_data = true;
                break;
            }
        }
    }
    
    if (m_stk_dir.size() > 0)
    {
        Log::info("AssetsAndroid", "Data files found in: %s", 
                  m_stk_dir.c_str());
    }

    // Create data dir if it's not available anywhere
    if (m_stk_dir.size() == 0)
    {
        std::string preferred_path = getPreferredPath(paths);

        if (preferred_path.length() > 0)
        {
            if (m_file_manager->checkAndCreateDirectoryP(preferred_path + "/" +
                                                        app_dir_name + "/data"))
            {
                Log::info("AssetsAndroid", "Data directory created in: %s",
                          preferred_path.c_str());
                m_stk_dir = preferred_path + "/" + app_dir_name;
                needs_extract_data = true;
            }
        }
    }

    // If getPreferredPath failed for some reason, then try to use the first
    // available path
    if (m_stk_dir.size() == 0)
    {
        for (std::string path : paths)
        {
            if (m_file_manager->checkAndCreateDirectoryP(path + "/" + 
                                                        app_dir_name + "/data"))
            {
                Log::info("AssetsAndroid", "Data directory created in: %s",
                          path.c_str());
                m_stk_dir = path + "/" + app_dir_name;
                needs_extract_data = true;
                break;
            }
        }
    }

    // We can't continue if STK dir has not been found
    if (m_stk_dir.size() == 0)
    {
        Log::fatal("AssetsAndroid", "Fatal error: Couldn't find Supertuxkart "
                   "data directory");
    }

    // Check if assets were extracted properly
    if (!m_file_manager->fileExists(m_stk_dir + "/.extracted") &&
        !needs_extract_data)
    {
        needs_extract_data = true;
        Log::warn("AssetsAndroid", "Assets seem to be not extracted properly, "
                  "because the .extracted file doesn't exist. Force "
                  "extracting assets...");
    }

    if (!m_file_manager->checkAndCreateDirectoryP(m_stk_dir + "/home"))
    {
        Log::warn("AssetsAndroid", "Couldn't create home directory");
    }

    // Set some useful variables
    setenv("SUPERTUXKART_DATADIR", m_stk_dir.c_str(), 1);
    setenv("HOME", (m_stk_dir + "/home").c_str(), 1);
    setenv("XDG_CONFIG_HOME", (m_stk_dir + "/home").c_str(), 1);

    // Extract data directory from apk if it's needed
    if (needs_extract_data)
    {
        m_progress_bar = new ProgressBarAndroid();
        m_progress_bar->draw(0.01f);
        
        if (hasAssets())
        {
            removeData();
            extractData();
            
            if (!m_file_manager->fileExists(m_stk_dir + "/.extracted"))
            {
                Log::fatal("AssetsAndroid", "Fatal error: Assets were not "
                           "extracted properly");
            }
        }
        
        delete m_progress_bar;

    }

#endif
}

//-----------------------------------------------------------------------------
/** A function that extracts whole data directory from apk file to a real
 *  path in the filesystem
 */
void AssetsAndroid::extractData()
{
#ifdef ANDROID
    const std::string dirs_list = "directories.txt";

    bool success = true;

    // Create .nomedia file
    touchFile(m_stk_dir + "/.nomedia");

    // Extract base directory first, so that we will be able to open the file
    // with dir names
    success = extractDir("");

    if (!success)
    {
        Log::error("AssetsAndroid", "Error: Couldn't extract main directory.");
        return;
    }

    std::fstream file(m_stk_dir + "/" + dirs_list, std::ios::in);

    if (file.good())
    {
        unsigned int lines_count = 0;

        while (!file.eof())
        {
            std::string dir_name;
            getline(file, dir_name);

            if (dir_name.length() == 0 || dir_name.at(0) == '#')
                continue;

            lines_count++;
        }

        if (lines_count > 0)
        {
            file.clear();
            file.seekg(0, std::ios::beg);

            unsigned int current_line = 1;

            while (!file.eof())
            {
                std::string dir_name;
                getline(file, dir_name);

                if (dir_name.length() == 0 || dir_name.at(0) == '#')
                    continue;

                success = extractDir(dir_name);

                assert(lines_count > 0);
                float pos = 0.01f + (float)(current_line) / lines_count * 0.99f;
                m_progress_bar->draw(pos);
                current_line++;

                if (m_progress_bar->closeEventReceived())
                {
                    success = false;
                }

                if (!success)
                    break;
            }
        }
    }
    else
    {
        Log::warn("AssetsAndroid", "Warning: Cannot open %s file. Ignoring "
                  "extraction of other directories.", dirs_list.c_str());
    }

    file.close();

    // Mark the extraction as successful if everything is ok
    if (success)
    {
        touchFile(m_stk_dir + "/.extracted");
    }
#endif
}

//-----------------------------------------------------------------------------
/** A function that extracts selected directory from apk file
 *  \param dir_name Directory to extract from assets
 *  \return True if successfully extracted
 */
bool AssetsAndroid::extractDir(std::string dir_name)
{
#ifdef ANDROID
    if (!global_android_app)
        return false;
        
    AAssetManager* amgr = global_android_app->activity->assetManager;

    Log::info("AssetsAndroid", "Extracting %s directory",
              dir_name.length() > 0 ? dir_name.c_str() : "main");

    std::string output_dir = dir_name;

    if (m_stk_dir.length() > 0)
    {
        output_dir = m_stk_dir + "/" + dir_name;
    }

    AAssetDir* asset_dir = AAssetManager_openDir(amgr, dir_name.c_str());

    if (asset_dir == NULL)
    {
        Log::warn("AssetsAndroid", "Couldn't get asset dir: %s",
                  dir_name.c_str());
        return true;
    }

    bool dir_created = m_file_manager->checkAndCreateDirectory(output_dir);
    
    if (!dir_created)
    {
        dir_created = m_file_manager->checkAndCreateDirectoryP(output_dir);
    }
    
    if (!dir_created)
    {
        Log::warn("AssetsAndroid", "Couldn't create %s directory",
                  output_dir.c_str());
        return false;
    }

    const int buf_size = 65536;
    char* buf = new char[buf_size]();
    bool extraction_failed = false;

    while (!extraction_failed)
    {
        const char* filename = AAssetDir_getNextFileName(asset_dir);

        // Check if finished
        if (filename == NULL)
            break;

        if (strlen(filename) == 0)
            continue;

        std::string file_path = std::string(filename);

        if (dir_name.length() > 0)
        {
            file_path = dir_name + "/" + std::string(filename);
        }

        AAsset* asset = AAssetManager_open(amgr, file_path.c_str(),
                                           AASSET_MODE_STREAMING);

        if (asset == NULL)
        {
            Log::warn("AssetsAndroid", "Asset is null: %s", filename);
            continue;
        }

        std::string output_path = output_dir + "/" + std::string(filename);
        std::fstream out_file(output_path, std::ios::out | std::ios::binary);

        if (!out_file.good())
        {
            extraction_failed = true;
            Log::error("AssetsAndroid", "Couldn't create a file: %s", filename);
            AAsset_close(asset);
            break;
        }

        int nb_read = 0;
        while ((nb_read = AAsset_read(asset, buf, buf_size)) > 0)
        {
            out_file.write(buf, nb_read);

            if (out_file.fail())
            {
                extraction_failed = true;
                break;
            }
        }

        out_file.close();

        if (out_file.fail() || extraction_failed)
        {
            extraction_failed = true;
            Log::error("AssetsAndroid", "Extraction failed for file: %s",
                      filename);
        }

        AAsset_close(asset);
    }

    delete[] buf;

    AAssetDir_close(asset_dir);

    return !extraction_failed;
#endif

    return false;
}

//-----------------------------------------------------------------------------
/** A function that removes whole STK data directory
 */
void AssetsAndroid::removeData()
{
#ifdef ANDROID
    if (m_stk_dir.length() == 0)
        return;
        
    std::string app_dir_name = ANDROID_APP_DIR_NAME;

    // Make sure that we are not accidentally removing wrong directory
    if (m_stk_dir.find("/" + app_dir_name) == std::string::npos &&
        m_stk_dir.find("/stk") == std::string::npos)
    {
        Log::error("AssetsAndroid", "Invalid data directory: %s",
                   m_stk_dir.c_str());
        assert(false);
        return;
    }

    std::set<std::string> files;
    m_file_manager->listFiles(files, m_stk_dir, true);

    for (std::string file : files)
    {
        if (file == m_stk_dir + "/." || file == m_stk_dir + "/..")
            continue;

        // Don't delete home directory that contains configuration files
        // and add-ons
        if (file == m_stk_dir + "/home")
            continue;

        // Don't delete .nomedia file. It has a sense to keep it for home
        // directory, i.e. for textures of add-on karts etc.
        if (file == m_stk_dir + "/.nomedia")
            continue;

        Log::info("AssetsAndroid", "Deleting file: %s", file.c_str());

        if (m_file_manager->isDirectory(file))
        {
            m_file_manager->removeDirectory(file);
        }
        else
        {
            m_file_manager->removeFile(file);
        }
    }

    std::string data_path = getDataPath();
    
    if (!data_path.empty())
    {
        const std::vector<std::string> child_paths = 
        {
            data_path + "/files/libchildprocess.so",
            data_path + "/files/libchildprocess_ai.so"
        };
    
        for (auto child_path : child_paths)
        {
            if (!m_file_manager->fileExists(child_path))
                continue;
                
            Log::info("AssetsAndroid", "Deleting old childprocess: %s", 
                      child_path.c_str());
            m_file_manager->removeFile(child_path);
        }
    }
#endif
}

//-----------------------------------------------------------------------------
/** A function that checks if assets are included in the package
 *  \return true if apk has assets
 */
bool AssetsAndroid::hasAssets()
{
#ifdef ANDROID
    AAssetManager* amgr = global_android_app->activity->assetManager;
    
    AAsset* asset = AAssetManager_open(amgr, "has_assets.txt",
                                       AASSET_MODE_STREAMING);

    if (asset == NULL)
    {
        Log::info("AssetsAndroid", "Package doesn't have assets");
        return false;
    }

    Log::info("AssetsAndroid", "Package has assets");
    AAsset_close(asset);
    return true;
#endif

    return false;
}

//-----------------------------------------------------------------------------
/** A function that creates empty file
 *  \param path A path to the file that should be created
 */
void AssetsAndroid::touchFile(std::string path)
{
#ifdef ANDROID
    if (m_file_manager->fileExists(path))
        return;

    std::fstream file(path, std::ios::out | std::ios::binary);

    if (!file.good())
    {
        Log::warn("AssetsAndroid", "Error: Cannot create %s file.",
                  path.c_str());
    }

    file.close();
#endif
}

//-----------------------------------------------------------------------------
/** Checks if there is write access for selected path
 *  \param path A path that should be checked
 *  \return True if there is write access
 */
bool AssetsAndroid::isWritable(std::string path)
{
#ifdef ANDROID
    return access(path.c_str(), R_OK) == 0 && access(path.c_str(), W_OK) == 0;
#endif

    return false;
}

//-----------------------------------------------------------------------------
/** Determines best path for extracting assets, depending on available disk
 *  space.
 *  \param paths A list of paths that should be checked
 *  \return Best path or empty string in case of error
 */
std::string AssetsAndroid::getPreferredPath(const std::vector<std::string>& 
                                            paths)
{
#ifdef ANDROID
    std::string preferred_path;
    int prev_available_space = 0;

    for (std::string path : paths)
    {
        // Paths that start with /data should be used only as a fallback if
        // everything other doesn't work, because typical user doesn't have
        // access to these directories and i.e. can't manually delete the files
        // to clean up device
        if (path.find("/data") == 0)
            continue;
            
        if (!isWritable(path))
            continue;

        struct statfs stat;

        if (statfs(path.c_str(), &stat) != 0)
            continue;

        int available_space = (int)((stat.f_bavail * stat.f_bsize) / 1000000);

        Log::info("AssetsAndroid", "Available space in '%s': %i MB",
                  path.c_str(), available_space);
                  
        if (available_space > prev_available_space)
        {
            preferred_path = path;
            prev_available_space = available_space;
        }
    }

    return preferred_path;
#endif

    return "";
}

//-----------------------------------------------------------------------------
/** Get a path for internal data directory
 *  \return Path for internal data directory or empty string when failed
 */
std::string AssetsAndroid::getDataPath()
{
#ifdef ANDROID
    std::string data_path = "/data/data/" ANDROID_PACKAGE_NAME;
    
    if (access(data_path.c_str(), R_OK) != 0)
    {
        Log::warn("AssetsAndroid", "Cannot use standard data dir");
        
        if (global_android_activity)
        {
            AndroidApplicationInfo application_info = 
                CIrrDeviceAndroid::getApplicationInfo(global_android_activity);
            
            data_path = application_info.data_dir;
        }
        
        if (access(data_path.c_str(), R_OK) != 0)
        {
            data_path = "";
        }
    }
    
    return data_path;
#endif

    return "";
}

//-----------------------------------------------------------------------------
/** Get a path for internal lib directory
 *  \return Path for internal lib directory or empty string when failed
 */
std::string AssetsAndroid::getLibPath()
{
#ifdef ANDROID
    std::string lib_path;
    
    if (global_android_activity)
    {
        AndroidApplicationInfo application_info = 
            CIrrDeviceAndroid::getApplicationInfo(global_android_activity);
    
        lib_path = application_info.native_lib_dir;
    }

    if (access(lib_path.c_str(), R_OK) != 0)
    {
        lib_path = "";
    }

    return lib_path;
#endif

    return "";
}
