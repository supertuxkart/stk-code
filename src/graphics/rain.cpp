//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013  Joerg Henrichs, Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/material.hpp"
#include "graphics/per_camera_node.hpp"
#include "graphics/rain.hpp"
#include "graphics/shaders.hpp"
#include "modes/world.hpp"
#include "states_screens/race_gui.hpp"
#include "utils/constants.hpp"
#include "utils/random_generator.hpp"

#include <ISceneManager.h>
#include <SMeshBuffer.h>

using namespace video;
using namespace scene;
using namespace core;

// OPENGL PARTICLE SYSTEM
#include <ICameraSceneNode.h>
#include "../source/Irrlicht/COpenGLExtensionHandler.h"
#include "io/file_manager.hpp"

#ifdef _IRR_WINDOWS_API_
#define IRR_OGL_LOAD_EXTENSION(X) wglGetProcAddress(reinterpret_cast<const char*>(X))
#else
#include <GL/glx.h>
#define IRR_OGL_LOAD_EXTENSION(X) glXGetProcAddress(reinterpret_cast<const GLubyte*>(X))
#endif


PFNGLGENTRANSFORMFEEDBACKSPROC glGenTransformFeedbacks;
PFNGLBINDTRANSFORMFEEDBACKPROC glBindTransformFeedback;
PFNGLDRAWTRANSFORMFEEDBACKPROC glDrawTransformFeedback;
PFNGLBEGINTRANSFORMFEEDBACKPROC glBeginTransformFeedback;
PFNGLENDTRANSFORMFEEDBACKPROC glEndTransformFeedback;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glTransformFeedbackVaryings;
PFNGLBINDBUFFERBASEPROC glBindBufferBase;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
#ifdef _IRR_WINDOWS_API_
PFNGLACTIVETEXTUREPROC glActiveTexture;
#endif
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;

void initGL()
{
    glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC)IRR_OGL_LOAD_EXTENSION("glGenTransformFeedbacks");
    glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glBindTransformFeedback");
    glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glDrawTransformFeedback");
    glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glBeginTransformFeedback");
    glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)IRR_OGL_LOAD_EXTENSION("glEndTransformFeedback");
    glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)IRR_OGL_LOAD_EXTENSION("glBindBufferBase");
    glGenBuffers = (PFNGLGENBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glGenBuffers");
    glBindBuffer = (PFNGLBINDBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glBindBuffer");
    glBufferData = (PFNGLBUFFERDATAPROC)IRR_OGL_LOAD_EXTENSION("glBufferData");
    glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribPointer");
    glCreateShader = (PFNGLCREATESHADERPROC)IRR_OGL_LOAD_EXTENSION("glCreateShader");
    glCompileShader = (PFNGLCOMPILESHADERPROC)IRR_OGL_LOAD_EXTENSION("glCompileShader");
    glShaderSource = (PFNGLSHADERSOURCEPROC)IRR_OGL_LOAD_EXTENSION("glShaderSource");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glCreateProgram");
    glAttachShader = (PFNGLATTACHSHADERPROC)IRR_OGL_LOAD_EXTENSION("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC)IRR_OGL_LOAD_EXTENSION("glUseProgram");
    glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)IRR_OGL_LOAD_EXTENSION("glEnableVertexAttribArray");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)IRR_OGL_LOAD_EXTENSION("glGetUniformLocation");
    glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)IRR_OGL_LOAD_EXTENSION("glUniformMatrix4fv");
    glUniform1f = (PFNGLUNIFORM1FPROC)IRR_OGL_LOAD_EXTENSION("glUniform1f");
    glUniform3f = (PFNGLUNIFORM3FPROC)IRR_OGL_LOAD_EXTENSION("glUniform3f");
    glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)IRR_OGL_LOAD_EXTENSION("glDisableVertexAttribArray");
    glDeleteShader = (PFNGLDELETESHADERPROC)IRR_OGL_LOAD_EXTENSION("glDeleteShader");
    glGetShaderiv = (PFNGLGETSHADERIVPROC)IRR_OGL_LOAD_EXTENSION("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetShaderInfoLog");
#ifdef _IRR_WINDOWS_API_
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glActiveTexture");
#endif
    glUniform2f = (PFNGLUNIFORM2FPROC)IRR_OGL_LOAD_EXTENSION("glUniform2f");
    glUniform1i = (PFNGLUNIFORM1IPROC)IRR_OGL_LOAD_EXTENSION("glUniform1i");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramInfoLog");
    glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)IRR_OGL_LOAD_EXTENSION("glTransformFeedbackVaryings");
}

// Mostly from shader tutorial
GLuint LoadShader(const char * file, unsigned type) {
    GLuint Id = glCreateShader(type);
    std::string Code;
    std::ifstream Stream(file, std::ios::in);
    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            Code += "\n" + Line;
        Stream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    printf("Compiling shader : %s\n", file);
    char const * SourcePointer = Code.c_str();
    int length = strlen(SourcePointer);
    glShaderSource(Id, 1, &SourcePointer, &length);
    glCompileShader(Id);

    glGetShaderiv(Id, GL_COMPILE_STATUS, &Result);
    if (Result == GL_FALSE) {
        glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetShaderInfoLog(Id, InfoLogLength, NULL, ErrorMessage);
        printf(ErrorMessage);
        delete[] ErrorMessage;
    }

    return Id;
}

GLuint LoadProgram(const char * vertex_file_path, const char * fragment_file_path){
    GLuint VertexShaderID = LoadShader(vertex_file_path, GL_VERTEX_SHADER);
    GLuint FragmentShaderID = LoadShader(fragment_file_path, GL_FRAGMENT_SHADER);

    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE) {
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, ErrorMessage);
        printf(ErrorMessage);
        delete[] ErrorMessage;
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount){
  GLuint Shader = LoadShader(vertex_file_path, GL_VERTEX_SHADER);
  GLuint Program = glCreateProgram();
  glAttachShader(Program, Shader);
  glTransformFeedbackVaryings(Program, varyingscount, varyings, GL_INTERLEAVED_ATTRIBS);
  glLinkProgram(Program);

  GLint Result = GL_FALSE;
  int InfoLogLength;
  glGetProgramiv(Program, GL_LINK_STATUS, &Result);
  if (Result == GL_FALSE) {
      glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
      char *ErrorMessage = new char[InfoLogLength];
      glGetProgramInfoLog(Program, InfoLogLength, NULL, ErrorMessage);
      printf(ErrorMessage);
      delete[] ErrorMessage;
  }
  glDeleteShader(Shader);
  return Program;
}

GLuint getTextureGLuint(ITexture *tex) { return static_cast<const COpenGLTexture*>(tex)->getOpenGLTextureName(); }

void bindUniformToTextureUnit(GLuint location, GLuint texid, unsigned textureUnit) {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texid);
    glUniform1i(location, textureUnit);
}

class GPUParticle {
private:
  GLuint SimulationProgram, RenderProgram, tfb_vertex_buffer[2];
  GLuint texture, normal_and_depth;
  GLuint loc_campos, loc_viewm, loc_time;
  GLuint loc_screenw, loc_screen, loc_invproj, texloc_tex, texloc_normal_and_depths;
  unsigned count;
public:
  GPUParticle(unsigned c, float *initialSamples, GLuint tex, GLuint rtt) :
      count(c), texture(tex), normal_and_depth(rtt) {
    initGL();
    glGenBuffers(2, tfb_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[0]);
    glBufferData(GL_ARRAY_BUFFER, 3 * count * sizeof(float), initialSamples, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[1]);
    glBufferData(GL_ARRAY_BUFFER, 3 * count * sizeof(float), 0, GL_STREAM_DRAW);

    RenderProgram = LoadProgram(file_manager->getAsset("shaders/rain.vert").c_str(), file_manager->getAsset("shaders/rain.frag").c_str());
    loc_screenw = glGetUniformLocation(RenderProgram, "screenw");
    loc_screen = glGetUniformLocation(RenderProgram, "screen");
    loc_invproj = glGetUniformLocation(RenderProgram, "invproj");
    texloc_tex = glGetUniformLocation(RenderProgram, "tex");
    texloc_normal_and_depths = glGetUniformLocation(RenderProgram, "normals_and_depth");

    const char *varyings[] = {"currentPosition"};
    SimulationProgram = LoadTFBProgram(file_manager->getAsset("shaders/rainsim.vert").c_str(), varyings, 1);
    loc_campos = glGetUniformLocation(SimulationProgram, "campos");
    loc_viewm = glGetUniformLocation(SimulationProgram, "viewm");
    loc_time = glGetUniformLocation(SimulationProgram, "time");
  }

  void simulate() {
    glUseProgram(SimulationProgram);
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 90.0f;
    const matrix4 viewm = irr_driver->getVideoDriver()->getTransform(ETS_VIEW);
    const vector3df campos = irr_driver->getSceneManager()->getActiveCamera()->getPosition();

    glEnable(GL_RASTERIZER_DISCARD);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfb_vertex_buffer[1]);

    glUniformMatrix4fv(loc_viewm, 1, GL_FALSE, viewm.pointer());
    glUniform1f(loc_time, time);
    glUniform3f(loc_campos, campos.X, campos.Y, campos.Z);
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, count);
    glEndTransformFeedback();
    glDisable(GL_RASTERIZER_DISCARD);
  }

  void render() {
    const float screenw = (float)UserConfigParams::m_width;

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glUseProgram(RenderProgram);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, tfb_vertex_buffer[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    float screen[2] = { (float)UserConfigParams::m_width,
        (float)UserConfigParams::m_height };
    matrix4 invproj = irr_driver->getVideoDriver()->getTransform(ETS_PROJECTION);
    invproj.makeInverse();

    bindUniformToTextureUnit(texloc_tex, texture, 0);
    bindUniformToTextureUnit(texloc_normal_and_depths, normal_and_depth, 1);

    glUniformMatrix4fv(loc_invproj, 1, GL_FALSE, invproj.pointer());
    glUniform2f(loc_screen, screen[0], screen[1]);
    glUniform1f(loc_screenw, screenw);
    glDrawArrays(GL_POINTS, 0, count);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
  }
};
// END OPENGL PARTICLE SYSTEM

// The actual rain node
class RainNode: public scene::ISceneNode
{
private:
    GPUParticle *gpupart;
public:
    RainNode(scene::ISceneManager* mgr, ITexture *tex)
            : scene::ISceneNode(0, mgr, -1)
    {
        mat.Lighting = false;
        mat.ZWriteEnable = false;
        mat.MaterialType = irr_driver->getShader(ES_RAIN);
        mat.Thickness = 200;
        mat.BlendOperation = EBO_ADD;

        mat.setTexture(0, tex);
        mat.TextureLayer[0].TextureWrapU =
        mat.TextureLayer[0].TextureWrapV = ETC_CLAMP_TO_EDGE;

        count = 2500;
        area = 3500;

        u32 i;
        float x, y, z, vertices[7500];
        for (i = 0; i < count; i++)
    {
            x = ((rand() % area) - area/2) / 100.0f;
            y = ((rand() % 2400)) / 100.0f;
            z = ((rand() % area) - area/2) / 100.0f;

            vertices[3 * i] = x;
            vertices[3 * i + 1] = y;
            vertices[3 * i + 2] = z;
        }
        gpupart = new GPUParticle(count, vertices, getTextureGLuint(mat.getTexture(0)), getTextureGLuint(irr_driver->getRTT(RTT_NORMAL_AND_DEPTH)));

        box.addInternalPoint(vector3df((float)(-area/2)));
        box.addInternalPoint(vector3df((float)( area/2)));
    }

    ~RainNode()
    {
        delete gpupart;
    }

    virtual void render()
    {
        gpupart->simulate();
        gpupart->render();
        // We need to force irrlicht to update its internal states
        IVideoDriver * const drv = irr_driver->getVideoDriver();
        drv->setMaterial(mat);
        static_cast<COpenGLDriver*>(drv)->setRenderStates3DMode();
    }

    virtual const core::aabbox3d<f32>& getBoundingBox() const
    {
        return box;
    }

    virtual void OnRegisterSceneNode()
            {
        if (IsVisible &&
           (irr_driver->getRenderPass() & ESNRP_TRANSPARENT) == ESNRP_TRANSPARENT)
        {
            SceneManager->registerNodeForRendering(this, ESNRP_TRANSPARENT);
            }

        ISceneNode::OnRegisterSceneNode();
        }

    virtual u32 getMaterialCount() const { return 1; }
    virtual video::SMaterial& getMaterial(u32 i) { return mat; }

private:
    video::SMaterial mat;
    core::aabbox3d<f32> box;
    u32 count;
    s32 area;
};

// The rain manager

Rain::Rain(Camera *camera, irr::scene::ISceneNode* parent)
{
    m_lightning = camera->getIndex()==0;

    if (m_lightning)
        m_thunder_sound = sfx_manager->createSoundSource("thunder");

    Material* m = material_manager->getMaterial("rain.png");
    assert(m != NULL);

    RandomGenerator g;
    m_next_lightning = (float)g.get(35);

    RainNode *node = new RainNode(irr_driver->getSceneManager(), m->getTexture());
    m_node = irr_driver->addPerCameraNode(node, camera->getCameraSceneNode(), parent);
    m_node->setAutomaticCulling(0);
}   // Rain

// ----------------------------------------------------------------------------

Rain::~Rain()
{
    m_node->drop();      // drop STK's reference
    m_node->remove();    // Then remove it from the scene graph.

    if (m_lightning && m_thunder_sound != NULL) sfx_manager->deleteSFX(m_thunder_sound);
}

// ----------------------------------------------------------------------------

void Rain::update(float dt)
{
    if (m_lightning)
    {
        m_next_lightning -= dt;

        if (m_next_lightning < 0.0f)
        {
            RaceGUIBase* gui_base = World::getWorld()->getRaceGUI();
            if (gui_base != NULL)
            {
                gui_base->doLightning();
                if (m_thunder_sound) m_thunder_sound->play();
            }

            RandomGenerator g;
            m_next_lightning = 35 + (float)g.get(35);
        }
    }

}   // update

// ----------------------------------------------------------------------------

void Rain::setPosition(const core::vector3df& position)
{
    m_node->getChild()->setPosition(position);
}   // setPosition

// ----------------------------------------------------------------------------

void Rain::setCamera(scene::ICameraSceneNode* camera)
{
    m_node->setCamera(camera);
}
