#ifndef HEADER_SDLDRV_H
#define HEADER_SDLDRV_H

#include <SDL.h>

void initSDL (int videoFlags);
void pollEvents();
void keyboardInput (const SDL_keysym& key);
void shutdown();

#endif

/* EOF */
