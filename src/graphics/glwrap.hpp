//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef SERVER_ONLY

#ifndef GLWRAP_HEADER_H
#define GLWRAP_HEADER_H

#include "graphics/gl_headers.hpp"

#include "utils/log.hpp"
#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/vec3.hpp"

#include <irrlicht.h>
#include <vector>

namespace HardwareStats
{
    class Json;
}

void initGL();

class GPUTimer;

class ScopedGPUTimer
{
protected:
    GPUTimer &timer;
public:
    ScopedGPUTimer(GPUTimer &);
    ~ScopedGPUTimer();
};

class GPUTimer
{
    friend class ScopedGPUTimer;
    GLuint query;
    bool initialised;
    unsigned lastResult;
    bool canSubmitQuery;
public:
    GPUTimer();
    unsigned elapsedTimeus();
};

class FrameBuffer : public NoCopy
{
private:
    GLuint fbo, fbolayer;
    std::vector<GLuint> RenderTargets;
    GLuint DepthTexture;
    unsigned int width, height;
public:
    FrameBuffer();
    FrameBuffer(const std::vector <GLuint> &RTTs, unsigned int w, unsigned int h, bool layered = false);
    FrameBuffer(const std::vector <GLuint> &RTTs, GLuint DS, unsigned int w, unsigned int h, bool layered = false);
    ~FrameBuffer();
    void bind() const;
    void bindLayer(unsigned) const;
    const std::vector<GLuint> &getRTT() const { return RenderTargets; }
    GLuint getDepthTexture() const { assert(DepthTexture); return DepthTexture; }
    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }
    static void Blit(const FrameBuffer &Src, const FrameBuffer &Dst, GLbitfield mask = GL_COLOR_BUFFER_BIT, GLenum filter = GL_NEAREST);
    void BlitToDefault(size_t, size_t, size_t, size_t);

    LEAK_CHECK();
};

class VertexUtils
{
public:
    static void bindVertexArrayAttrib(enum video::E_VERTEX_TYPE tp)
    {
        switch (tp)
        {
        case video::EVT_STANDARD:
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            // Color
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            // Texcoord
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            break;
        case video::EVT_2TCOORDS:
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            // Color
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            // Texcoord
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            // SecondTexcoord
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)36);
            break;
        case video::EVT_TANGENTS:
            // Position
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            // Normal
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            // Color
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            // Texcoord
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            // Tangent
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)36);
            // Bitangent
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)48);
            break;
        case video::EVT_SKINNED_MESH:
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), 0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)12);
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(tp), (GLvoid*)24);
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)28);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)44);
            glEnableVertexAttribArray(5);
            glVertexAttribIPointer(5, 4, GL_SHORT, getVertexPitchFromType(tp), (GLvoid*)60);
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_HALF_FLOAT, GL_FALSE, getVertexPitchFromType(tp), (GLvoid*)68);
            break;
        }
    }
};

void draw3DLine(const core::vector3df& start,
    const core::vector3df& end, irr::video::SColor color);

bool hasGLExtension(const char* extension);
const std::string getGLExtensions();
void getGLLimits(HardwareStats::Json *json);
bool checkGLError();

#endif

#endif   // supertuxkart

