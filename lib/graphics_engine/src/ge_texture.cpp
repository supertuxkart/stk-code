#include "ge_main.hpp"
#include "ge_dx9_texture.hpp"
#include "ge_gl_texture.hpp"
#include "ge_vulkan_texture.hpp"
#include "ge_texture.hpp"

#include <IFileSystem.h>
#include <IVideoDriver.h>

namespace GE
{
using namespace irr;
video::IImage* getResizedImage(const std::string& path,
                               const core::dimension2du& max_size,
                               core::dimension2d<u32>* orig_size)
{
    io::IReadFile* file =
        getDriver()->getFileSystem()->createAndOpenFile(path.c_str());
    if (file == NULL)
        return NULL;
    video::IImage* image = getResizedImage(file, max_size, orig_size);
    file->drop();
    return image;
}   // getResizedImage

// ----------------------------------------------------------------------------
core::dimension2du getResizingTarget(const core::dimension2du& orig_size,
                                     const core::dimension2du& max_size)
{
    bool has_npot = !getGEConfig()->m_disable_npot_texture &&
        getDriver()->queryFeature(video::EVDF_TEXTURE_NPOT);

    core::dimension2du tex_size = orig_size.getOptimalSize(!has_npot);
    if (tex_size.Width > max_size.Width)
        tex_size.Width = max_size.Width;
    if (tex_size.Height > max_size.Height)
        tex_size.Height = max_size.Height;
    return tex_size;
}   // getResizingTarget

// ----------------------------------------------------------------------------
video::IImage* getResizedImageFullPath(const io::path& fullpath,
                                       const core::dimension2d<u32>& max_size,
                                       core::dimension2d<u32>* orig_size,
                                     const core::dimension2d<u32>* target_size)
{
    io::IReadFile* file = io::createReadFile(fullpath);
    if (file == NULL)
        return NULL;
    video::IImage* texture_image = getResizedImage(file, max_size, orig_size,
        target_size);
    file->drop();
    return texture_image;
}   // getResizedImageFullPath

// ----------------------------------------------------------------------------
video::IImage* getResizedImage(irr::io::IReadFile* file,
                               const core::dimension2du& max_size,
                               core::dimension2d<u32>* orig_size,
                               const core::dimension2d<u32>* target_size)
{
    video::IImage* image = getDriver()->createImageFromFile(file);
    if (image == NULL)
        return NULL;
    if (orig_size)
        *orig_size = image->getDimension();

    core::dimension2du img_size = image->getDimension();
    core::dimension2du tex_size;
    if (target_size)
        tex_size = *target_size;
    else
        tex_size = getResizingTarget(img_size, max_size);

    if (image->getColorFormat() != video::ECF_A8R8G8B8 ||
        tex_size != img_size)
    {
        video::IImage* new_texture = getDriver()->createImage(
            video::ECF_A8R8G8B8, tex_size);
        if (tex_size != img_size)
            image->copyToScaling(new_texture);
        else
            image->copyTo(new_texture);
        image->drop();
        return new_texture;
    }

    return image;
}   // getResizedImage

// ----------------------------------------------------------------------------
irr::video::ITexture* createTexture(const std::string& path,
    std::function<void(irr::video::IImage*)> image_mani)
{
    switch (GE::getDriver()->getDriverType())
    {
    case video::EDT_OPENGL:
    case video::EDT_OGLES2:
        return new GEGLTexture(path, image_mani);
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
    case video::EDT_DIRECT3D9:
        return new GEDX9Texture(path, image_mani);
#endif
    case video::EDT_VULKAN:
        return new GEVulkanTexture(path, image_mani);
    default:
        return NULL;
    }
}   // createTexture

// ----------------------------------------------------------------------------
irr::video::ITexture* createTexture(video::IImage* img,
                                    const std::string& name)
{
    switch (GE::getDriver()->getDriverType())
    {
    case video::EDT_OPENGL:
    case video::EDT_OGLES2:
        return new GEGLTexture(img, name);
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
    case video::EDT_DIRECT3D9:
        return new GEDX9Texture(img, name);
#endif
    case video::EDT_VULKAN:
        return new GEVulkanTexture(img, name);
    default:
        return NULL;
    }
}   // createTexture

// ----------------------------------------------------------------------------
irr::video::ITexture* createFontTexture(const std::string& name,
                                        unsigned size, bool single_channel)
{
    switch (GE::getDriver()->getDriverType())
    {
    case video::EDT_OPENGL:
    case video::EDT_OGLES2:
        return new GEGLTexture(name, size, single_channel);
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_
    case video::EDT_DIRECT3D9:
        return new GEDX9Texture(name, size);
#endif
    case video::EDT_VULKAN:
        return new GEVulkanTexture(name, size, single_channel);
    default:
        return NULL;
    }
}   // createFontTexture

}
