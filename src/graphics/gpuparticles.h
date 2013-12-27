#ifndef GPUPARTICLES_H
#define GPUPARTICLES_H

#ifndef _IRR_WINDOWS_API_
#define GL_GLEXT_PROTOTYPES 1
#endif
#include "graphics/glwrap.hpp"
#include <ISceneManager.h>

void initGL();
GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path);
GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount);
GLuint getTextureGLuint(irr::video::ITexture *tex);
void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit);


class GPUParticle {
private:
    unsigned count;
    GLuint SimulationProgram, RenderProgram, tfb_vertex_buffer[2];
    GLuint texture, normal_and_depth;
    GLuint loc_campos, loc_viewm, loc_time;
    GLuint loc_screenw, loc_screen, loc_invproj, texloc_tex, texloc_normal_and_depths;
public:
    GPUParticle(unsigned c, float *initialSamples, GLuint tex, GLuint rtt);
    void simulate();
    void render();
};

#endif // GPUPARTICLES_H