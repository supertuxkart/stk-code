#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdexcept>
#include <sstream>
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

