#ifndef IMAGE_H
#define IMAGE_H

#include <SDL.h>
#include <SDL_ttf.h>

#include "gui_glext.h"

//previously in config.h:

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK 0xFF000000
#define GMASK 0x00FF0000
#define BMASK 0x0000FF00
#define AMASK 0x000000FF
#else
#define RMASK 0x000000FF
#define GMASK 0x0000FF00
#define BMASK 0x00FF0000
#define AMASK 0xFF000000
#endif

/*---------------------------------------------------------------------------*/

void   image_snap(char *);
void   image_size(int *, int *, int, int);

void         image_swab (SDL_Surface *);
void         image_white(SDL_Surface *);
SDL_Surface *image_scale(SDL_Surface *, int);

GLuint make_image_from_surf(int *, int *, SDL_Surface *);
GLuint make_image_from_file(int *, int *,
                            int *, int *, const char *);
GLuint make_image_from_font(int *, int *,
                            int *, int *, const char *, TTF_Font *);

/*---------------------------------------------------------------------------*/

#endif
