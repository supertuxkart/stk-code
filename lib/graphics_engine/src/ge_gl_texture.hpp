#ifndef HEADER_GE_GL_TEXTURE_HPP
#define HEADER_GE_GL_TEXTURE_HPP

#include "glad/gl.h"

#include <functional>
#include <string>
#include <ITexture.h>

using namespace irr;

namespace GE
{
class GEGLTexture : public video::ITexture
{
private:
    core::dimension2d<u32> m_size, m_orig_size, m_max_size;

    std::function<void(video::IImage*)> m_image_mani;

    uint8_t* m_locked_data;

    GLuint m_texture_name;

    unsigned int m_texture_size;

    const video::E_DRIVER_TYPE m_driver_type;

    const bool m_disable_reload;

    bool m_single_channel;

    // ------------------------------------------------------------------------
    void upload(uint8_t* data);
    // ------------------------------------------------------------------------
    void formatConversion(uint8_t* data, unsigned int* format, unsigned int w,
                          unsigned int h) const;

public:
    // ------------------------------------------------------------------------
    GEGLTexture(const std::string& path,
        std::function<void(video::IImage*)> image_mani = nullptr);
    // ------------------------------------------------------------------------
    GEGLTexture(video::IImage* img, const std::string& name);
    // ------------------------------------------------------------------------
    GEGLTexture(const std::string& name, unsigned int size,
                bool single_channel);
    // ------------------------------------------------------------------------
    virtual ~GEGLTexture();
    // ------------------------------------------------------------------------
    virtual void* lock(video::E_TEXTURE_LOCK_MODE mode =
                       video::ETLM_READ_WRITE, u32 mipmap_level = 0);
    // ------------------------------------------------------------------------
    virtual void unlock()
    {
        if (m_locked_data)
        {
            delete [] m_locked_data;
            m_locked_data = NULL;
        }
    }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getOriginalSize() const
                                                        { return m_orig_size; }
    // ------------------------------------------------------------------------
    virtual const core::dimension2d<u32>& getSize() const    { return m_size; }
    // ------------------------------------------------------------------------
    virtual video::E_DRIVER_TYPE getDriverType() const
                                                      { return m_driver_type; }
    // ------------------------------------------------------------------------
    virtual video::ECOLOR_FORMAT getColorFormat() const
                                                { return video::ECF_A8R8G8B8; }
    // ------------------------------------------------------------------------
    virtual u32 getPitch() const                                  { return 0; }
    // ------------------------------------------------------------------------
    virtual bool hasMipMaps() const        { return glGenerateMipmap != NULL; }
    // ------------------------------------------------------------------------
    virtual void regenerateMipMapLevels(void* mipmap_data = NULL)            {}
    // ------------------------------------------------------------------------
    virtual u64 getTextureHandler() const            { return m_texture_name; }
    // ------------------------------------------------------------------------
    virtual unsigned int getTextureSize() const      { return m_texture_size; }
    // ------------------------------------------------------------------------
    virtual void reload();
    // ------------------------------------------------------------------------
    virtual void updateTexture(void* data, irr::video::ECOLOR_FORMAT format,
                               u32 w, u32 h, u32 x, u32 y);
};   // GEGLTexture

}

#endif
