#ifndef HEADER_GE_TEXTURE_HPP
#define HEADER_GE_TEXTURE_HPP

#include <functional>
#include <string>
#include <ITexture.h>
#include <IImage.h>
#include <IReadFile.h>

namespace GE
{
irr::video::ITexture* createFontTexture(const std::string& name,
                                        unsigned size, bool single_channel);
irr::video::ITexture* createTexture(irr::video::IImage* img,
                                    const std::string& name);
irr::core::dimension2d<irr::u32> getResizingTarget(
                           const irr::core::dimension2d<irr::u32>& orig_size,
                           const irr::core::dimension2d<irr::u32>& max_size);
irr::video::IImage* getResizedImage(const std::string& path,
                           const irr::core::dimension2d<irr::u32>& max_size,
                           irr::core::dimension2d<irr::u32>* orig_size = NULL);
irr::video::IImage* getResizedImageFullPath(const irr::io::path& fullpath,
                           const irr::core::dimension2d<irr::u32>& max_size,
                           irr::core::dimension2d<irr::u32>* orig_size = NULL,
                   const irr::core::dimension2d<irr::u32>* target_size = NULL);
irr::video::IImage* getResizedImage(irr::io::IReadFile* file,
                           const irr::core::dimension2d<irr::u32>& max_size,
                           irr::core::dimension2d<irr::u32>* orig_size = NULL,
                   const irr::core::dimension2d<irr::u32>* target_size = NULL);
irr::video::ITexture* createTexture(const std::string& path,
    std::function<void(irr::video::IImage*)> image_mani = nullptr);
};   // GE

#endif
