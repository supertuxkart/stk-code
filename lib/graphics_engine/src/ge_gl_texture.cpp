#include "ge_gl_texture.hpp"
#include "ge_main.hpp"
#include "ge_texture.hpp"

namespace GE
{
GEGLTexture::GEGLTexture(const std::string& path,
                         std::function<void(video::IImage*)> image_mani)
           : video::ITexture(path.c_str()), m_image_mani(image_mani),
             m_single_channel(false), m_texture_name(0), m_texture_size(0),
             m_driver_type(GE::getDriver()->getDriverType()),
             m_disable_reload(false)
{
    reload();
}   // GEGLTexture

// ----------------------------------------------------------------------------
GEGLTexture::GEGLTexture(video::IImage* img, const std::string& name)
           : video::ITexture(name.c_str()), m_image_mani(nullptr),
             m_single_channel(false), m_texture_name(0), m_texture_size(0),
             m_driver_type(GE::getDriver()->getDriverType()),
             m_disable_reload(true)
{
    glGenTextures(1, &m_texture_name);
    m_size = m_orig_size = img->getDimension();
    uint8_t* data = (uint8_t*)img->lock();
    upload(data);
    img->unlock();
    img->drop();
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
        &m_orig_size);
    if (texture_image == NULL)
        return;
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
    if (m_driver_type == video::EDT_OGLES2 || !glGetTexImage)
    {
        video::IImage* img = getResizedImage(NamedPath.getPtr());
        img->setDeleteMemory(false);
        void* data = img->lock();
        img->drop();
        return data;
    }
    uint8_t* pixels = new uint8_t[m_size.Width * m_size.Height * 4]();
    GLint tmp_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tmp_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture_name);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, tmp_texture);
    return pixels;
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

}
