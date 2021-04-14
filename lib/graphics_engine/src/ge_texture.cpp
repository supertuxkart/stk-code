#include "ge_main.hpp"
#include "ge_texture.hpp"

#include <IVideoDriver.h>
#include <IAttributes.h>

namespace GE
{
using namespace irr;
video::IImage* getResizedImage(const std::string& path)
{
    video::IImage* image = getDriver()->createImageFromFile(path.c_str());
    if (image == NULL)
        return NULL;

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
}

}
