#include "MoltenVK.h"
#include "SDL_vulkan.h"
#include "vulkan_wrapper.h"

#include <dlfcn.h>
#import <AppKit/NSApplication.h>

#ifdef DLOPEN_MOLTENVK
PFN_vkGetPhysicalDeviceMetalFeaturesMVK vkGetPhysicalDeviceMetalFeaturesMVK = NULL;

namespace irr
{
// ----------------------------------------------------------------------------
MoltenVK::MoltenVK()
{
    m_loaded = false;
    m_handle = NULL;
    // MacOSX 10.11 or later supports Metal (MoltenVK)
    if (floor(NSAppKitVersionNumber) <= NSAppKitVersionNumber10_10_Max)
        return;
    const char* paths[3] =
    {
        // STK release binary path after dylibbundler
        "@executable_path/../libs/libMoltenVK.dylib",
        // bin/supertuxkart.app/Contents/MacOS/supertuxkart
        "@executable_path/../../../../../dependencies-macosx/lib/libMoltenVK.dylib",
        "NULL"
    };
    for (int i = 0; i < 3; i++)
    {
        if (SDL_Vulkan_LoadLibrary(paths[i]) == 0)
        {
            // Following SDL/src/loadso/dlopen/SDL_sysloadso.c
            m_handle = dlopen(paths[i], RTLD_NOW|RTLD_LOCAL);
            if (!m_handle)
            {
                SDL_Vulkan_UnloadLibrary();
                return;
            }
            vkGetPhysicalDeviceMetalFeaturesMVK = (PFN_vkGetPhysicalDeviceMetalFeaturesMVK)
                dlsym(m_handle, "vkGetPhysicalDeviceMetalFeaturesMVK");
            if (!vkGetPhysicalDeviceMetalFeaturesMVK)
            {
                SDL_Vulkan_UnloadLibrary();
                return;
            }
            m_loaded = true;
            break;
        }
    }
}   // MoltenVK

// ----------------------------------------------------------------------------
MoltenVK::~MoltenVK()
{
    if (m_loaded)
    {
        dlclose(m_handle);
        SDL_Vulkan_UnloadLibrary();
    }
}   // ~MoltenVK

#endif

}
