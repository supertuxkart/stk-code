#ifndef HEADER_PHYSFSLOADER_H
#define HEADER_PHYSFSLOADER_H

#include <plib/ssg.h>
#include <string>
#include <vector>
#include <set>

class Loader : public ssgLoaderOptions
{
public:
    Loader();
    ~Loader();

    virtual void makeModelPath(char* path, const char* fname) const;
    virtual void makeTexturePath(char* path, const char* fname) const;

    std::string getPath(const std::string& name) const;
    void listFiles(std::set<std::string>& result, const std::string& dir)
        const;

    void addSearchPath(const std::string& path);
    void initConfigDir();

private:
    void make_path(char* path, const char* dir, const char* fname) const;

    std::vector<std::string> searchPath;
};

void registerImageLoaders();

extern Loader* loader;

#endif

