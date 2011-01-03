//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin
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
#ifdef ADDONS_MANAGER
#include "irrlicht.h"
#include <string.h>
#include <iostream>
#include <fstream> 

#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"

using namespace irr;
using namespace io;
s32 IFileSystem_copyFileToFile(IWriteFile* dst, IReadFile* src)
{
  char buf[1024];
  const s32 sz = sizeof(buf) / sizeof(*buf);

  s32 rx = src->getSize();
  for (s32 r = 0; r < rx; /**/)
  {
    s32 wx = src->read(buf, sz);
    for (s32 w = 0; w < wx; /**/)
    {
      s32 n = dst->write(buf + w, wx - w);
      if (n < 0)
        return -1;
      else
        w += n;
    }
    r += wx;
  }
  return rx;
}   // IFileSystem_copyFileToFile

// ----------------------------------------------------------------------------
/** Extracts all files from the zip archive 'from' to the directory 'to'.
 *  \param from A zip archive.
 *  \param to The destination directory.
 */
bool extract_zip(const std::string &from, const std::string &to)
{
    //Add the zip to the file system
    IFileSystem *file_system = irr_driver->getDevice()->getFileSystem();
    file_system->addZipFileArchive(from.c_str());

    // Get the recently added archive, which is necessary to get a 
    // list of file in the zip archive.
    io::IFileArchive *zip_archive = 
        file_system->getFileArchive(file_system->getFileArchiveCount()-1);
    const io::IFileList *zip_file_list = zip_archive->getFileList();
    // Copy all files from the zip archive to the destination
    for(unsigned int i=0; i<zip_file_list->getFileCount(); i++)
    {
        const std::string current_file=zip_file_list->getFileName(i).c_str();
        std::cout << current_file << std::endl;
        IReadFile* srcFile  = file_system->createAndOpenFile(current_file.c_str());
        IWriteFile* dstFile = 
            file_system->createAndWriteFile((to+"/"+current_file).c_str());
        if (IFileSystem_copyFileToFile(dstFile, srcFile) < 0)
        {
            printf("Could not copy '%s' from archive '%s'.\n",
                    current_file.c_str(), from.c_str());
            printf("This is ignored, but the addon might not work.\n");
        }
        srcFile->drop();
        dstFile->drop();
    }
    // Remove the zip from the filesystem to save memory and avoid 
    // problem with a name conflict. Note that we have to convert
    // the path using getAbsolutePath, otherwise windows name
    // will not be detected correctly (e.g. if from=c:\...  the
    // stored filename will be c:/..., which then does not match
    // on removing it. getAbsolutePath will convert all \ to /.
    file_system->removeFileArchive(file_system->getAbsolutePath(from.c_str()));
    
    return true;
}   // extract_zip
#endif
