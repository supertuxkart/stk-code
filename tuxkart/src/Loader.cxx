#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdexcept>
#include <sstream>
#include <SDL.h>
#include <SDL_image.h>
#include "Loader.h"

Loader* loader = 0;

Loader::Loader()
{
}

Loader::~Loader()
{
}

void
Loader::make_path(char* path, const char* dir, const char* fname) const
{
    struct stat mystat;
    
    for(std::vector<std::string>::const_iterator i = searchPath.begin();
            i != searchPath.end(); ++i) {
        sprintf(path, "%s/%s/%s", i->c_str(), dir, fname);
        // convert backslashes to slashes
        size_t len = strlen(path);
        for(size_t i = 0; i < len; ++i)
            if(path[i] == '\\')
                path[i] = '/';

        if(stat(path, &mystat) < 0)
            continue;

        return;
    }
}

void
Loader::makeModelPath(char* path, const char* fname) const
{
    make_path(path, getModelDir(), fname);
}

void
Loader::makeTexturePath(char* path, const char* fname) const
{
    make_path(path, getTextureDir(), fname);
}

void
Loader::addSearchPath(const std::string& path)
{
    searchPath.push_back(path);
}

void
Loader::initConfigDir()
{
#ifdef WIN32
	/*nothing*/
#else
	/*if HOME environment variable exists
	create directory $HOME/.tuxkart*/
	if(getenv("HOME")!=NULL)
	{
            std::string pathname;
            pathname = getenv("HOME");
            pathname += "/.tuxkart";
	    mkdir(pathname.c_str(), 0755);
	}
#endif
}

std::string
Loader::getPath(const std::string& fname) const
{
    struct stat mystat;
    std::string result;
   
    for(std::vector<std::string>::const_iterator i = searchPath.begin();
            i != searchPath.end(); ++i) {
        result = *i;
        result += '/';
        result += fname;

        if(stat(result.c_str(), &mystat) < 0)
            continue;

        return result;
    }

    std::stringstream msg;
    msg << "Couldn't find file '" << fname << "'.";
    throw std::runtime_error(msg.str());
}

void
Loader::listFiles(std::set<std::string>& result, const std::string& dir)
    const
{
    struct stat mystat;

#ifdef DEBUG
    // don't list directories with a slash on the end, it'll fail on win32
    assert(dir[dir.size()-1] != '/');
#endif

    result.clear();
    for(std::vector<std::string>::const_iterator i = searchPath.begin();
            i != searchPath.end(); ++i) {
        std::string path = *i;

	path += '/';
        path += dir;

        if(stat(path.c_str(), &mystat) < 0)
            continue;
        if(! S_ISDIR(mystat.st_mode))
            continue;

        DIR* mydir = opendir(path.c_str());
        if(!mydir)
            continue;

        struct dirent* mydirent;
        while( (mydirent = readdir(mydir)) != 0) {
            result.insert(mydirent->d_name);
        }
        closedir(mydir);
    }
}

// PNG and JPEG loader (with help of SDL_Image)

bool ssgLoadIMG(const char* fname, ssgTextureInfo* info)
{
    SDL_Surface* surface = IMG_Load(fname);
    if(!surface) {
        ulSetError(UL_WARNING, "ssgLoadPNG: Failed to load '%s': %s",
                fname, SDL_GetError());
        return false;
    }
    if(surface->pitch != surface->w * surface->format->BytesPerPixel) {
        ulSetError(UL_WARNING, "ssgLoadPNG: incompatible surface while "
                "loading '%s' (pitch mismatch)", fname);
        return false;
    }

    if(info != 0) {
        info->width = surface->w;
        info->height = surface->h;
        info->depth = surface->format->BytesPerPixel;
        info->alpha = surface->format->BytesPerPixel > 3;
    }

    // create mipmaps
    int bpp = surface->format->BytesPerPixel;

    // grrr, ssgMakeMipMaps calls delete[] on the data, it also seems to need
    // the texture upsidedown...
    size_t datasize = surface->w * surface->h * bpp;
    GLubyte* data = new GLubyte[datasize];
    GLubyte* dest = data + surface->pitch * (surface->h - 1);
    GLubyte* src = (GLubyte*) surface->pixels;
    for(int i = 0; i < surface->h; ++i) {
        memcpy(dest, src, surface->pitch);
        dest -= surface->pitch;
        src += surface->pitch;
    }
        
    bool result = ssgMakeMipMaps(data, surface->w, surface->h, bpp);
    SDL_FreeSurface(surface);

    return result;
}

void registerImageLoaders()
{
    ssgAddTextureFormat(".png", ssgLoadIMG);
    ssgAddTextureFormat(".jpg", ssgLoadIMG);
    ssgAddTextureFormat(".jpeg", ssgLoadIMG);    
}
