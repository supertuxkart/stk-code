#ifndef HEADER_SDLDRV_H
#define HEADER_SDLDRV_H

#include <SDL.h>

extern Uint8 *keyState;

void initVideo (bool fullscreen);
void pollEvents();
void keyboardInput (const SDL_keysym& key);
void shutdown();
void swapBuffers();

#endif

/* EOF */
