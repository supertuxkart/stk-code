#include "ge_gl_texture.hpp"
#include "ge_gl_utils.hpp"
#include "ge_main.hpp"
#include "ge_texture.hpp"

#include <IAttributes.h>
#include <vector>

// TODO remove it after vulkan is done
namespace irr
{
    namespace video { extern bool useCoreContext; }
}

namespace GE
{
GEGLTexture::GEGLTexture(const std::string& path,
                         std::function<void(video::IImage*)> image_mani)
           : video::ITexture(path.c_str()), m_image_mani(image_mani),
             m_locked_data(NULL), m_texture_name(0), m_texture_size(0),
             m_driver_type(GE::getDriver()->getDriverType()),
             m_disable_reload(false), m_single_channel(false)
{
    m_max_size = getDriver()->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    reload();
}   // GEGLTexture

// ----------------------------------------------------------------------------
GEGLTexture::GEGLTexture(video::IImage* img, const std::string& name)
           : video::ITexture(name.c_str()), m_image_mani(nullptr),
             m_locked_data(NULL), m_texture_name(0), m_texture_size(0),
             m_driver_type(GE::getDriver()->getDriverType()),
             m_disable_reload(true), m_single_channel(false)
{
    if (!img)
    {
        LoadingFailed = true;
        return;
    }
    glGenTextures(1, &m_texture_name);
    m_size = m_orig_size = img->getDimension();
    uint8_t* data = (uint8_t*)img->lock();
    upload(data);
    img->unlock();
    img->drop();
}   // GEGLTexture

// ----------------------------------------------------------------------------
GEGLTexture::GEGLTexture(const std::string& name, unsigned int size,
                         bool single_channel)
           : video::ITexture(name.c_str()), m_image_mani(nullptr),
             m_locked_data(NULL), m_texture_name(0), m_texture_size(0),
             m_driver_type(GE::getDriver()->getDriverType()),
             m_disable_reload(true), m_single_channel(false)
{
    glGenTextures(1, &m_texture_name);
    m_orig_size.Width = size;
    m_orig_size.Height = size;
    m_size = m_orig_size;

    bool texture_swizzle = false;
    if (m_driver_type == video::EDT_OGLES2)
    {
        int gl_major_version = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
        if (gl_major_version >=3)
            texture_swizzle = true;
        else
            texture_swizzle = false;
    }
    else
    {
        texture_swizzle = irr::video::useCoreContext &&
            GE::hasGLExtension("GL_ARB_texture_swizzle");
    }

    if (single_channel && texture_swizzle)
        m_single_channel = true;
    std::vector<uint8_t> data;
    data.resize(size * size * (m_single_channel ? 1 : 4), 0);
    upload(data.data());
}   // GEGLTexture

// ----------------------------------------------------------------------------
GEGLTexture::~GEGLTexture()
{
    if (m_texture_name != 0)
        glDeleteTextures(1, &m_texture_name);
}   // ~GEGLTexture

// ----------------------------------------------------------------------------
void GEGLTexture::reload()
{
    if (m_disable_reload)
        return;
    video::IImage* texture_image = getResizedImage(NamedPath.getPtr(),
        m_max_size, &m_orig_size);
    if (texture_image == NULL)
    {
        LoadingFailed = true;
        return;
    }
    m_size = texture_image->getDimension();
    if (m_image_mani)
        m_image_mani(texture_image);
    if (m_texture_name == 0)
        glGenTextures(1, &m_texture_name);
    uint8_t* data = (uint8_t*)texture_image->lock();
    upload(data);
    texture_image->unlock();
    texture_image->drop();
}   // reload

// ----------------------------------------------------------------------------
void GEGLTexture::upload(uint8_t* data)
{
    const unsigned int w = m_size.Width;
    const unsigned int h = m_size.Height;
    unsigned int format = m_single_channel ? GL_RED : GL_BGRA;
    unsigned int internal_format = m_single_channel ? GL_R8 : GL_RGBA8;

    if (m_driver_type == video::EDT_OGLES2)
    {
        formatConversion(data, &format, w, h);
        int gl_major_version = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
        // GLES 2.0 specs doesn't allow GL_RGBA8 internal format
        if (gl_major_version < 3)
            internal_format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    if (m_single_channel)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, format,
        GL_UNSIGNED_BYTE, data);
    if (hasMipMaps())
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_texture_size = w * h * (m_single_channel ? 1 : 4);
}   // upload

// ----------------------------------------------------------------------------
void* GEGLTexture::lock(video::E_TEXTURE_LOCK_MODE mode, u32 mipmap_level)
{
    if (mode != video::ETLM_READ_ONLY)
        return NULL;

    if (m_driver_type == video::EDT_OGLES2 || !glGetTexImage)
    {
        video::IImage* img = getResizedImage(NamedPath.getPtr(), m_max_size);
        if (!img)
            return NULL;
        img->setDeleteMemory(false);
        m_locked_data = (uint8_t*)img->lock();
        img->unlock();
        img->drop();
        return m_locked_data;
    }
    m_locked_data = new uint8_t[m_size.Width * m_size.Height * 4]();
    GLint tmp_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tmp_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, m_locked_data);
    glBindTexture(GL_TEXTURE_2D, tmp_texture);
    return m_locked_data;
}   // lock

//-----------------------------------------------------------------------------
void GEGLTexture::formatConversion(uint8_t* data, unsigned int* format,
                                   unsigned int w, unsigned int h) const
{
    if (!m_single_channel)
    {
        if (format)
            *format = GL_RGBA;
        for (unsigned int i = 0; i < w * h; i++)
        {
            uint8_t tmp_val = data[i * 4];
            data[i * 4] = data[i * 4 + 2];
            data[i * 4 + 2] = tmp_val;
        }
    }
}   // formatConversion

//-----------------------------------------------------------------------------
void GEGLTexture::updateTexture(void* data, video::ECOLOR_FORMAT format, u32 w,
                                u32 h, u32 x, u32 y)
{
    glBindTexture(GL_TEXTURE_2D, m_texture_name);

    if (m_single_channel)
    {
        if (format == video::ECF_R8)
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED,
                GL_UNSIGNED_BYTE, data);
        }
    }
    else
    {
        if (format == video::ECF_R8)
        {
            const unsigned int size = w * h;
            std::vector<uint8_t> image_data(size * 4, 255);
            uint8_t* orig_data = (uint8_t*)data;
            for (unsigned int i = 0; i < size; i++)
                image_data[4 * i + 3] = orig_data[i];
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA,
                GL_UNSIGNED_BYTE, image_data.data());
        }
        else if (format == video::ECF_A8R8G8B8)
        {
            uint8_t* u8_data = (uint8_t*)data;
            for (unsigned int i = 0; i < w * h; i++)
            {
                uint8_t tmp_val = u8_data[i * 4];
                u8_data[i * 4] = u8_data[i * 4 + 2];
                u8_data[i * 4 + 2] = tmp_val;
            }
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA,
                GL_UNSIGNED_BYTE, u8_data);
        }
    }
    if (hasMipMaps())
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}   // updateTexture

}
