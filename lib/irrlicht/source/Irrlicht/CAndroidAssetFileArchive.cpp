// Copyright (C) 2002-2011 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef   _IRR_COMPILE_ANDROID_ASSET_READER_

#include "CAndroidAssetReader.h"

#include "CReadFile.h"
#include "coreutil.h"
#include "CAndroidAssetFileArchive.h"
#include "CIrrDeviceAndroid.h"

#include <android_native_app_glue.h>
#include <android/native_activity.h>
#include <android/log.h>

namespace irr
{
namespace io
{

CAndroidAssetFileArchive *createAndroidAssetFileArchive(bool ignoreCase, bool ignorePaths)
{
    if(!CIrrDeviceAndroid::getAndroidApp())
        return NULL;
    return new CAndroidAssetFileArchive(ignoreCase, ignorePaths);
}


CAndroidAssetFileArchive::CAndroidAssetFileArchive(bool ignoreCase, bool ignorePaths)
  : CFileList("/asset", ignoreCase, ignorePaths)
{
  AssetManager = CIrrDeviceAndroid::getAndroidApp()->activity->assetManager;
  addDirectory("");
}


CAndroidAssetFileArchive::~CAndroidAssetFileArchive()
{
}


//! get the archive type
E_FILE_ARCHIVE_TYPE CAndroidAssetFileArchive::getType() const
{
	return EFAT_ANDROID_ASSET;
}

const IFileList* CAndroidAssetFileArchive::getFileList() const
{
    // The assert_manager can not read directory names, so
    // for now getFileList does not work as expected.
    return this;   // Keep the compiler happy
}


//! opens a file by file name
IReadFile* CAndroidAssetFileArchive::createAndOpenFile(const io::path& filename)
{
    CAndroidAssetReader *reader = new CAndroidAssetReader(filename);

    if(reader->isOpen())
        return reader;

    reader->drop();
    return NULL;
}

//! opens a file by index
IReadFile* CAndroidAssetFileArchive::createAndOpenFile(u32 index)
{
    // Since we can't list files, not much sense in giving them an index
}

void CAndroidAssetFileArchive::addDirectory(const io::path &dirname)
{
 
  AAssetDir *dir = AAssetManager_openDir(AssetManager, core::stringc(dirname).c_str());
  if(!dir)
    return;

  addItem(dirname, 0, 0, /*isDir*/true, getFileCount());
  while(const char *filename = AAssetDir_getNextFileName(dir))
  {
    core::stringc full_filename= dirname=="" ? filename
                                             : dirname+"/"+filename;

    // We can't get the size without opening the file - so for performance
    // reasons we set the file size to 0.
    addItem(full_filename, /*offet*/0, /*size*/0, /*isDir*/false, 
	    getFileCount());
  }
}

} // end namespace io
} // end namespace irr

#endif //  _IRR_COMPILE_ANDROID_ASSET_READER_
