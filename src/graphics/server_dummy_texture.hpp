#ifndef HEADER_SERVER_DUMMY_TEXTURE_HPP
#define HEADER_SERVER_DUMMY_TEXTURE_HPP

#include "utils/no_copy.hpp"

#include <string>
#include <ITexture.h>

using namespace irr;

class ServerDummyTexture : public video::ITexture, NoCopy
{
public:
    // ------------------------------------------------------------------------
    ServerDummyTexture(const std::string& p) : video::ITexture(p.c_str()) {}
    // ------------------------------------------------------------------------
    virtual ~ServerDummyTexture() {}
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0)
                                                               { return NULL; }
    // ------------------------------------------------------------------------
    virtual void unlock() {}
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
    {
        static core::dimension2d<u32> dummy;
        return dummy;
    }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const
    {
        return getOriginalSize();
    }
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
    {
        return video::EDT_NULL;
    }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const { return false; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u64 getTextureHandler() const                         { return 1; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const                   { return 0; }
};   // ServerDummyTexture

#endif
