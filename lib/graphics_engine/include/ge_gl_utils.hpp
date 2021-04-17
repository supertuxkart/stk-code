#ifndef HEADER_GE_GL_UTILS_HPP
#define HEADER_GE_GL_UTILS_HPP

#include <glad/gl.h>
#include <set>
#include <sstream>
#include <string>

namespace GE
{
inline bool hasGLExtension(const std::string& extension)
{
    if (glGetStringi)
    {
        int num = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num);
        for (int i = 0; i < num; i++)
        {
            char* ext = (char*)glGetStringi(GL_EXTENSIONS, i);
            if (ext && extension == ext)
                return true;
        }
        return false;
    }
    static std::set<std::string> extensions;
    if (extensions.empty())
    {
        char* all_ext = (char*)glGetString(GL_EXTENSIONS);
        if (all_ext)
        {
            std::stringstream ss(all_ext);
            while (true)
            {
                std::string ext;
                if (ss >> ext)
                    extensions.insert(ext);
                else
                    break;
            }
        }
    }
    return extensions.find(extension) != extensions.end();
}   // hasGLExtension

}

#endif
