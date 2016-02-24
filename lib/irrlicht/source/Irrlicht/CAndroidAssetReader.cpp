// Copyright (C) 2002-2011 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "IrrCompileConfig.h"

#ifdef   _IRR_COMPILE_ANDROID_ASSET_READER_

#include "CAndroidAssetReader.h"

#include "CReadFile.h"
#include "coreutil.h"
#include "CAndroidAssetReader.h"
#include "CIrrDeviceAndroid.h"

#include <android_native_app_glue.h>
#include <android/native_activity.h>
#include <android/log.h>

namespace irr
{
namespace io
{

CAndroidAssetReader::CAndroidAssetReader(const io::path &filename)
{
  AssetManager = CIrrDeviceAndroid::getAndroidApp()->activity->assetManager;
  Asset        = AAssetManager_open(AssetManager, 
			            core::stringc(filename).c_str(),
				    AASSET_MODE_RANDOM);
  Filename     = filename;

}

CAndroidAssetReader::~CAndroidAssetReader()
{
  if(Asset)
    AAsset_close(Asset);
}

s32 CAndroidAssetReader::read(void* buffer, u32 sizeToRead)
{
  return AAsset_read(Asset, buffer, sizeToRead);
}
      
bool CAndroidAssetReader::seek(long finalPos, bool relativeMovement)
{
  long off = AAsset_seek(Asset, finalPos, relativeMovement ? SEEK_CUR
			                                   : SEEK_SET);
  return off = relativeMovement-1;
}

long CAndroidAssetReader::getSize() const
{
  return AAsset_getLength(Asset);
}
      
long CAndroidAssetReader::getPos() const
{
  return AAsset_getLength(Asset) - AAsset_getRemainingLength(Asset);
}
      
const io::path& CAndroidAssetReader::getFileName() const
{
  return Filename;
}


} // end namespace io
} // end namespace irr

#endif //  _IRR_COMPILE_ANDROID_ASSET_READER_
