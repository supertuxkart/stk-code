#include "MoltenVK.h"
#include "SDL_vulkan.h"
#import <AppKit/NSApplication.h>

#ifdef DLOPEN_MOLTENVK

namespace irr
{
// ----------------------------------------------------------------------------
MoltenVK::MoltenVK()
{
    m_loaded = false;
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
            m_loaded = true;
            break;
        }
    }
}   // MoltenVK

// ----------------------------------------------------------------------------
MoltenVK::~MoltenVK()
{
    if (m_loaded)
        SDL_Vulkan_UnloadLibrary();
}   // ~MoltenVK

#endif

}
