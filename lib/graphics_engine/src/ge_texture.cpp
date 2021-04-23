#include "ge_main.hpp"
#include "ge_dx9_texture.hpp"
#include "ge_gl_texture.hpp"
#include "ge_texture.hpp"

#include <IVideoDriver.h>
#include <IAttributes.h>

namespace GE
{
using namespace irr;
video::IImage* getResizedImage(const std::string& path,
                               core::dimension2d<u32>* orig_size)
{
    video::IImage* image = getDriver()->createImageFromFile(path.c_str());
    if (image == NULL)
        return NULL;
    if (orig_size)
        *orig_size = image->getDimension();

    core::dimension2du img_size = image->getDimension();
    bool has_npot = !getGEConfig()->m_disable_npot_texture &&
        getDriver()->queryFeature(video::EVDF_TEXTURE_NPOT);

    core::dimension2du tex_size = img_size.getOptimalSize(!has_npot);
    const core::dimension2du& max_size = getDriver()->getDriverAttributes().
        getAttributeAsDimension2d("MAX_TEXTURE_SIZE");

    if (tex_size.Width > max_size.Width)
        tex_size.Width = max_size.Width;
    if (tex_size.Height > max_size.Height)
        tex_size.Height = max_size.Height;

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
    default:
        return NULL;
    }
}   // createFontTexture

}
