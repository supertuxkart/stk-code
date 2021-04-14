#ifndef HEADER_GE_TEXTURE_HPP
#define HEADER_GE_TEXTURE_HPP

#include <string>
#include <ITexture.h>
#include <IImage.h>

namespace GE
{
irr::video::ITexture* createFontTexture(const std::string& name);
irr::video::ITexture* createTexture(irr::video::IImage* img);
irr::video::IImage* getResizedImage(const std::string& path);
irr::video::ITexture* createTexture(const std::string& path);
};   // GE

#endif
