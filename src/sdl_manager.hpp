#ifndef SDL_MANAGER_HPP
#define SDL_MANAGER_HPP

namespace SDLManager
{
    void init();
    // void setVideoMode(bool resetTextures);
    void toggleFullscreen(bool resetTextures);
    void showPointer();
    void hidePointer();
}

#endif

