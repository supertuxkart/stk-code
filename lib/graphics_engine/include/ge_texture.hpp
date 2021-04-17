#ifndef HEADER_GE_TEXTURE_HPP
#define HEADER_GE_TEXTURE_HPP

#include <functional>
#include <string>
#include <ITexture.h>
#include <IImage.h>

namespace GE
{
irr::video::ITexture* createFontTexture(const std::string& name,
                                        unsigned size, bool single_channel);
irr::video::ITexture* createTexture(irr::video::IImage* img,
                                    const std::string& name);
irr::video::IImage* getResizedImage(const std::string& path,
                           irr::core::dimension2d<irr::u32>* orig_size = NULL);
irr::video::ITexture* createTexture(const std::string& path,
    std::function<void(irr::video::IImage*)> image_mani = nullptr);
};   // GE

#endif
