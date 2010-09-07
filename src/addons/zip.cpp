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
#include "graphics/irr_driver.hpp"
#include <string.h>
#include <iostream>
#include <fstream> 

using namespace irr;
using namespace io;
s32 IFileSystem_copyFileToFile(IWriteFile* dst, IReadFile* src)
{
  char buf[1024];
  const s32 sz = sizeof(buf) / sizeof(*buf);

  s32 r, rx = src->getSize();
  for (r = 0; r < rx; /**/)
  {
    s32 w, wx = src->read(buf, sz);
    for (w = 0; w < wx; /**/)
    {
      s32 n = dst->write(buf + w, wx - w);
      if (n < 0)
        return -1;
      else
        w += n;
    }
    r += w;
  }

  return r;
}

bool extract_zip(std::string from, std::string to)
{
    //get the stk irrlicht device
    IrrlichtDevice * device = irr_driver->getDevice();

    //get the filesystem from the device
    IFileSystem*  pfs = device->getFileSystem();
    //Add the zip to the file system
    pfs->addZipFileArchive(from.c_str());

    //IFileArchive * zipfile = pfs->getFileArchive(0);
    //extract the file where there is the others file name
    IReadFile* srcFile = pfs->createAndOpenFile("file_list");
    if (srcFile == NULL)
    {
        std::cerr << "Could not open 'file_list', sorry. @" 
                  << __FILE__ << ":" << __LINE__ << std::endl;
        return false;
    }
    IWriteFile* dstFile = pfs->createAndWriteFile(std::string(to + "file_list").c_str());
    if (dstFile == NULL)
    {
        std::cerr << "Could not create '" << std::string(to + "file_list").c_str() << "', sorry. @" 
                  << __FILE__ << ":" << __LINE__ << std::endl;
        if (srcFile != NULL) srcFile->drop();
        return false;
    }
    std::cout << from.c_str() << std::endl;
    //....
    if (IFileSystem_copyFileToFile(dstFile, srcFile) < 0)
    {
        std::cerr << "IFileSystem_copyFileToFile failed @" << __FILE__ << ":" << __LINE__ << std::endl;
        if (srcFile != NULL) srcFile->drop();
        if (dstFile != NULL) dstFile->drop();
        return false;
    }
    srcFile->drop();
    dstFile->drop();

    std::string file_list;
    std::string buff;
    std::ifstream entree(std::string(to + "file_list").c_str(), std::ios::in);
    while (entree >> buff)
        file_list += buff;
    std::cout << file_list << std::endl;
    std::string current_file;
    for(unsigned int i=0; i < file_list.size(); i++)
    {
        if(file_list.c_str()[i] != '\\')
        {
            current_file += file_list[i];
        }
        else
        {
            std::cout << current_file << std::endl;
                IReadFile* srcFile = pfs->createAndOpenFile(current_file.c_str());
                IWriteFile* dstFile = pfs->createAndWriteFile(std::string(to + current_file).c_str());
                if (IFileSystem_copyFileToFile(dstFile, srcFile) < 0)
                ; // error
                srcFile->drop();
                dstFile->drop();
            current_file = "";
        }
    }
    //remove the zip from the filesystem to save memory and avoid problem with a name conflict
    pfs->removeFileArchive(from.c_str());
    
    return true;
}
#endif
