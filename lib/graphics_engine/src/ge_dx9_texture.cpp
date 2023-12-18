#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_DIRECT3D_9_

#include "ge_dx9_texture.hpp"
#include "ge_main.hpp"
#include "ge_texture.hpp"

#include <IAttributes.h>
#include <vector>

namespace GE
{
GEDX9Texture::GEDX9Texture(const std::string& path,
                         std::function<void(video::IImage*)> image_mani)
           : video::ITexture(path.c_str()), m_image_mani(image_mani),
             m_device_9(NULL), m_texture_9(NULL), m_texture_size(0),
             m_disable_reload(false)
{
    m_max_size = getDriver()->getDriverAttributes()
        .getAttributeAsDimension2d("MAX_TEXTURE_SIZE");
    getDevice9();
    reload();
}   // GEDX9Texture

// ----------------------------------------------------------------------------
GEDX9Texture::GEDX9Texture(video::IImage* img, const std::string& name)
           : video::ITexture(name.c_str()), m_image_mani(nullptr),
             m_device_9(NULL), m_texture_9(NULL), m_texture_size(0),
             m_disable_reload(true)
{
    getDevice9();
    if (!m_device_9 || !img)
    {
        LoadingFailed = true;
        return;
    }
    uint8_t* data = NULL;
    m_size = m_orig_size = img->getDimension();
    HRESULT hr = m_device_9->CreateTexture(m_size.Width, m_size.Height,
        0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
        &m_texture_9, NULL);
    if (FAILED(hr))
    {
        LoadingFailed = true;
        goto exit;
    }
    data = (uint8_t*)img->lock();
    upload(data);
exit:
    img->unlock();
    img->drop();
}   // GEDX9Texture

// ----------------------------------------------------------------------------
GEDX9Texture::GEDX9Texture(const std::string& name, unsigned int size)
           : video::ITexture(name.c_str()), m_image_mani(nullptr),
             m_device_9(NULL), m_texture_9(NULL), m_texture_size(0),
             m_disable_reload(true)
{
    getDevice9();
    if (!m_device_9)
    {
        LoadingFailed = true;
        return;
    }
    m_orig_size.Width = size;
    m_orig_size.Height = size;
    m_size = m_orig_size;
    HRESULT hr = m_device_9->CreateTexture(m_size.Width, m_size.Height,
        0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
        &m_texture_9, NULL);
    if (FAILED(hr))
    {
        LoadingFailed = true;
        return;
    }
    std::vector<uint8_t> data;
    data.resize(size * size * 4, 0);
    upload(data.data());
}   // GEDX9Texture

// ----------------------------------------------------------------------------
GEDX9Texture::~GEDX9Texture()
{
    if (m_texture_9)
        m_texture_9->Release();
    if (m_device_9)
        m_device_9->Release();
}   // ~GEDX9Texture

// ----------------------------------------------------------------------------
void GEDX9Texture::getDevice9()
{
    m_device_9 = GE::getDriver()->getExposedVideoData().D3D9.D3DDev9;
    if (m_device_9)
        m_device_9->AddRef();
}   // getDevice9

// ----------------------------------------------------------------------------
void GEDX9Texture::reload()
{
    if (m_disable_reload)
        return;

    if (!m_device_9)
    {
        LoadingFailed = true;
        return;
    }

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
    if (m_texture_9 != NULL)
    {
        m_texture_9->Release();
        m_texture_9 = NULL;
    }
    uint8_t* data = (uint8_t*)texture_image->lock();
    HRESULT hr = m_device_9->CreateTexture(m_size.Width, m_size.Height,
        0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
        &m_texture_9, NULL);
    if (FAILED(hr))
    {
        LoadingFailed = true;
        goto exit;
    }
    upload(data);
exit:
    texture_image->unlock();
    texture_image->drop();
}   // reload

// ----------------------------------------------------------------------------
void GEDX9Texture::upload(uint8_t* data)
{
    const unsigned int w = m_size.Width;
    const unsigned int h = m_size.Height;
    HRESULT hr;
    D3DLOCKED_RECT rect;
    hr = m_texture_9->LockRect(0, &rect, 0, 0);
    if (FAILED(hr))
        return;
    uint8_t* dst = (uint8_t*)rect.pBits;
    for (u32 i = 0; i < h; i++)
    {
        memcpy(dst, data, w * 4);
        data += w * 4;
        dst += rect.Pitch;
    }
    hr = m_texture_9->UnlockRect(0);
    if (FAILED(hr))
        return;
    m_texture_9->GenerateMipSubLevels();
    m_texture_size = w * h * 4;
}   // upload

// ----------------------------------------------------------------------------
void* GEDX9Texture::lock(video::E_TEXTURE_LOCK_MODE mode, u32 mipmap_level)
{
    if (mode != video::ETLM_READ_ONLY || !m_texture_9)
        return NULL;
    HRESULT hr;
    D3DLOCKED_RECT rect;
    hr = m_texture_9->LockRect(0, &rect, 0,
        (mode == video::ETLM_READ_ONLY) ? D3DLOCK_READONLY : 0);
    if (FAILED(hr))
        return NULL;
    return rect.pBits;
}   // lock

//-----------------------------------------------------------------------------
void GEDX9Texture::updateTexture(void* data, video::ECOLOR_FORMAT format,
                                 u32 w, u32 h, u32 x, u32 y)
{
    if (!m_texture_9)
        return;

    std::vector<uint8_t> image_data;
    uint8_t* src = NULL;
    if (format == video::ECF_R8)
    {
        const unsigned int size = w * h;
        image_data.resize(size * 4, 255);
        uint8_t* orig_data = (uint8_t*)data;
        for (unsigned int i = 0; i < size; i++)
            image_data[4 * i + 3] = orig_data[i];
        src = image_data.data();
    }
    else if (format == video::ECF_A8R8G8B8)
    {
        src = (uint8_t*)data;
    }

    if (src == NULL)
        return;
    HRESULT hr;
    D3DLOCKED_RECT rect;
    RECT subimg;
    subimg.left = x;
    subimg.top = y;
    subimg.right = x + w;
    subimg.bottom = y + h;
    hr = m_texture_9->LockRect(0, &rect, &subimg, 0);
    if (FAILED(hr))
        return;
    uint8_t* dst = (uint8_t*)rect.pBits;
    for (u32 i = 0; i < h; i++)
    {
        memcpy(dst, src, w * 4);
        src += w * 4;
        dst += rect.Pitch;
    }
    hr = m_texture_9->UnlockRect(0);
    if (FAILED(hr))
        return;
    m_texture_9->GenerateMipSubLevels();
}   // updateTexture

}

#endif
