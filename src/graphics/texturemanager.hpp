#ifndef MEMORYMANAGER_HPP
#define MEMORYMANAGER_HPP

#include "gl_headers.hpp"
#include <ITexture.h>
#include <string>

GLuint getTextureGLuint(irr::video::ITexture *tex);
GLuint getDepthTexture(irr::video::ITexture *tex);
void resetTextureTable();
void compressTexture(irr::video::ITexture *tex, bool srgb, bool premul_alpha = false);
bool loadCompressedTexture(const std::string& compressed_tex);
void saveCompressedTexture(const std::string& compressed_tex);

#endif
