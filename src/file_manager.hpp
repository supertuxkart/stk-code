//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_FILE_MANAGER_H
#define HEADER_FILE_MANAGER_H

#include <string>
#include <vector>
#include <set>
#include "callback_manager.hpp"

class FileManager : public ssgLoaderOptions
{
private:
    bool                        m_is_full_path;
    std::string                 m_root_dir;
    std::vector<std::string>    m_texture_search_path,
                                m_model_search_path,
                                m_music_search_path;
    bool findFile               (std::string& full_path,
                                 const std::string& fname, 
                                 const std::vector<std::string>& search_path) const;
public:
    FileManager();
    ~FileManager();

    std::string getHomeDir      () const;
    std::string getTrackDir     () const;
    std::string getKartDir      () const;
    std::string getHerringDir   () const;
    std::vector<std::string>getMusicDirs() const;
    std::string getTextureFile  (const std::string& fname) const;
    std::string getKartFile     (const std::string& fname,
                                 const std::string& kart="") const;
    std::string getTrackFile    (const std::string& fname, 
                                 const std::string& track="") const;
    std::string getConfigFile   (const std::string& fname) const;
    std::string getHighscoreFile(const std::string& fname) const;
    std::string getLogFile      (const std::string& fname) const;
    std::string getHerringFile  (const std::string& fname) const;
    std::string getMusicFile    (const std::string& fname) const;
    std::string getSFXFile      (const std::string& fname) const;
    std::string getFontFile     (const std::string& fname) const;
    std::string getModelFile    (const std::string& fname) const;
#ifdef HAVE_GHOST_REPLAY
    std::string getReplayFile(const std::string& fname) const;
#endif

    void listFiles(std::set<std::string>& result, const std::string& dir,
                   bool is_full_path=false, bool make_full_path=false)
        const;

    void       pushTextureSearchPath(const std::string& path) 
                                    { m_texture_search_path.push_back(path);}
    void       pushModelSearchPath  (const std::string& path)
                                    { m_model_search_path.push_back(path);  }
    void       pushMusicSearchPath  (const std::string& path)
                                    { m_music_search_path.push_back(path);  }
    void       popTextureSearchPath () {m_texture_search_path.pop_back();   }
    void       popModelSearchPath   () {m_model_search_path.pop_back();     }
    void       popMusicSearchPath   () {m_music_search_path.pop_back();     }
    void       initConfigDir();
private:
    void         makePath     (std::string& path, const std::string& dir, 
                               const std::string& fname) const;
};

extern FileManager* file_manager;

#endif

