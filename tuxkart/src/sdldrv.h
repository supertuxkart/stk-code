#ifndef HEADER_SDLDRV_H
#define HEADER_SDLDRV_H

#include <SDL.h>

extern Uint8 *keyState;
extern SDL_Surface *sdl_screen;

void initVideo (int w, int h, bool fullscreen);
void pollEvents();
void keyboardInput (const SDL_keysym& key);
void shutdown();
void swapBuffers();
int  getScreenWidth();
int  getScreenHeight();

#endif

/* EOF */
