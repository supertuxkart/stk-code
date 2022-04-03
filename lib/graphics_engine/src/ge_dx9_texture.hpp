#ifndef HEADER_GE_DX9_TEXTURE_HPP
#define HEADER_GE_DX9_TEXTURE_HPP

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_

#include <d3d9.h>

#include <functional>
#include <string>
#include <ITexture.h>

using namespace irr;

namespace GE
{
class GEDX9Texture : public video::ITexture
{
private:
    core::dimension2d<u32> m_size, m_orig_size, m_max_size;

    std::function<void(video::IImage*)> m_image_mani;

    IDirect3DDevice9* m_device_9;

    IDirect3DTexture9* m_texture_9;

    unsigned int m_texture_size;

    const bool m_disable_reload;

    // ------------------------------------------------------------------------
    void getDevice9();
    // ------------------------------------------------------------------------
    void upload(uint8_t* data);

public:
    // ------------------------------------------------------------------------
    GEDX9Texture(const std::string& path,
        std::function<void(video::IImage*)> image_mani = nullptr);
    // ------------------------------------------------------------------------
    GEDX9Texture(video::IImage* img, const std::string& name);
    // ------------------------------------------------------------------------
    GEDX9Texture(const std::string& name, unsigned int size);
    // ------------------------------------------------------------------------
    virtual ~GEDX9Texture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0);
    // ------------------------------------------------------------------------
    virtual void unlock()
    {
        if (m_texture_9)
            m_texture_9->UnlockRect(0);
    }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
                                                        { return m_orig_size; }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const    { return m_size; }
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
                                               { return video::EDT_DIRECT3D9; }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const                            { return true; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u64 getTextureHandler() const          { return (u64)m_texture_9; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const      { return m_texture_size; }
    // ------------------------------------------------------------------------
    virtual void reload();
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y);
};   // GEDX9Texture

}

#endif

#endif
