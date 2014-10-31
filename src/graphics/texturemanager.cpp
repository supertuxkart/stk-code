#include "texturemanager.hpp"
#include <fstream>
#include <sstream>
#include "../../lib/irrlicht/source/Irrlicht/COpenGLTexture.h"
#include "irr_driver.hpp"


GLuint getTextureGLuint(irr::video::ITexture *tex)
{
    return static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName();
}

GLuint getDepthTexture(irr::video::ITexture *tex)
{
    assert(tex->isRenderTarget());
    return static_cast<irr::video::COpenGLFBOTexture*>(tex)->DepthBufferTexture;
}

static std::set<irr::video::ITexture *> AlreadyTransformedTexture;
static std::map<int, video::ITexture*> unicolor_cache;

void resetTextureTable()
{
    AlreadyTransformedTexture.clear();
    unicolor_cache.clear();
}

void compressTexture(irr::video::ITexture *tex, bool srgb, bool premul_alpha)
{
    if (AlreadyTransformedTexture.find(tex) != AlreadyTransformedTexture.end())
        return;
    AlreadyTransformedTexture.insert(tex);

    glBindTexture(GL_TEXTURE_2D, getTextureGLuint(tex));

    std::string cached_file;
    if (UserConfigParams::m_texture_compression)
    {
        // Try to retrieve the compressed texture in cache
        std::string tex_name = irr_driver->getTextureName(tex);
        if (!tex_name.empty()) {
            cached_file = file_manager->getTextureCacheLocation(tex_name) + ".gltz";
            if (!file_manager->fileIsNewer(tex_name, cached_file)) {
                if (loadCompressedTexture(cached_file))
                    return;
            }
        }
    }

    size_t w = tex->getSize().Width, h = tex->getSize().Height;
    unsigned char *data = new unsigned char[w * h * 4];
    memcpy(data, tex->lock(), w * h * 4);
    tex->unlock();
    unsigned internalFormat, Format;
    if (tex->hasAlpha())
        Format = GL_BGRA;
    else
        Format = GL_BGR;

    if (premul_alpha)
    {
        for (unsigned i = 0; i < w * h; i++)
        {
            float alpha = data[4 * i + 3];
            if (alpha > 0.)
                alpha = pow(alpha / 255.f, 1.f / 2.2f);
            data[4 * i] = (unsigned char)(data[4 * i] * alpha);
            data[4 * i + 1] = (unsigned char)(data[4 * i + 1] * alpha);
            data[4 * i + 2] = (unsigned char)(data[4 * i + 2] * alpha);
        }
    }

    if (!UserConfigParams::m_texture_compression)
    {
        if (srgb)
            internalFormat = (tex->hasAlpha()) ? GL_SRGB_ALPHA : GL_SRGB;
        else
            internalFormat = (tex->hasAlpha()) ? GL_RGBA : GL_RGB;
    }
    else
    {
        if (srgb)
            internalFormat = (tex->hasAlpha()) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
        else
            internalFormat = (tex->hasAlpha()) ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, Format, GL_UNSIGNED_BYTE, (GLvoid *)data);
    glGenerateMipmap(GL_TEXTURE_2D);
    delete[] data;

    if (UserConfigParams::m_texture_compression && !cached_file.empty())
    {
        // Save the compressed texture in the cache for later use.
        saveCompressedTexture(cached_file);
    }
}

//-----------------------------------------------------------------------------
/** Try to load a compressed texture from the given file name.
*   Data in the specified file need to have a specific format. See the
*   saveCompressedTexture() function for a description of the format.
*   \return true if the loading succeeded, false otherwise.
*   \see saveCompressedTexture
*/
bool loadCompressedTexture(const std::string& compressed_tex)
{
    std::ifstream ifs(compressed_tex.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        return false;

    int internal_format;
    int w, h;
    int size = -1;
    ifs.read((char*)&internal_format, sizeof(int));
    ifs.read((char*)&w, sizeof(int));
    ifs.read((char*)&h, sizeof(int));
    ifs.read((char*)&size, sizeof(int));

    if (ifs.fail() || size == -1)
        return false;

    char *data = new char[size];
    ifs.read(data, size);
    if (!ifs.fail())
    {
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internal_format,
            w, h, 0, size, (GLvoid*)data);
        glGenerateMipmap(GL_TEXTURE_2D);
        delete[] data;
        ifs.close();
        return true;
    }
    delete[] data;
    return false;
}

//-----------------------------------------------------------------------------
/** Try to save the last texture sent to glTexImage2D in a file of the given
*   file name. This function should only be used for textures sent to
*   glTexImage2D with a compressed internal format as argument.<br>
*   \note The following format is used to save the compressed texture:<br>
*         <internal-format><width><height><size><data> <br>
*         The first four elements are integers and the last one is stored
*         on \c size bytes.
*   \see loadCompressedTexture
*/
void saveCompressedTexture(const std::string& compressed_tex)
{
    int internal_format, width, height, size, compressionSuccessful;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internal_format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint *)&width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint *)&height);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, (GLint *)&compressionSuccessful);
    if (!compressionSuccessful)
        return;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, (GLint *)&size);

    char *data = new char[size];
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, (GLvoid*)data);
    std::ofstream ofs(compressed_tex.c_str(), std::ios::out | std::ios::binary);
    if (ofs.is_open())
    {
        ofs.write((char*)&internal_format, sizeof(int));
        ofs.write((char*)&width, sizeof(int));
        ofs.write((char*)&height, sizeof(int));
        ofs.write((char*)&size, sizeof(int));
        ofs.write(data, size);
        ofs.close();
    }
    delete[] data;
}

video::ITexture* getUnicolorTexture(const video::SColor &c)
{
    std::map<int, video::ITexture*>::iterator it = unicolor_cache.find(c.color);
    if (it != unicolor_cache.end())
    {
        it->second->grab();
        return it->second;
    }
    else
    {
        unsigned tmp[4] = {
            c.color,
            c.color,
            c.color,
            c.color
        };
        video::IImage *img = irr_driver->getVideoDriver()->createImageFromData(video::ECF_A8R8G8B8, core::dimension2d<u32>(2, 2), tmp);
        img->grab();
        std::stringstream name;
        name << "color" << c.color;
        video::ITexture* tex = irr_driver->getVideoDriver()->addTexture(name.str().c_str(), img);
        unicolor_cache[c.color] = tex;
        img->drop();
        return tex;
    }
}