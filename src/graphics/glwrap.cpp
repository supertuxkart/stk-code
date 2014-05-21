#include "graphics/glwrap.hpp"
#include "irr_driver.hpp"
#include <fstream>
#include <string>
#include "config/user_config.hpp"
#include "utils/profiler.hpp"

#ifdef _IRR_WINDOWS_API_
#define IRR_OGL_LOAD_EXTENSION(X) wglGetProcAddress(reinterpret_cast<const char*>(X))
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
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM3IPROC glUniform3i;
PFNGLUNIFORM4IPROC glUniform4i;
PFNGLUNIFORM1FVPROC glUniform1fv;
PFNGLUNIFORM4FVPROC glUniform4fv;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
PFNGLBLENDEQUATIONPROC glBlendEquation;
PFNGLVERTEXATTRIBDIVISORPROC glVertexAttribDivisor;
PFNGLDRAWARRAYSINSTANCEDPROC glDrawArraysInstanced;
PFNGLDRAWELEMENTSINSTANCEDPROC glDrawElementsInstanced;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
PFNGLTEXBUFFERPROC glTexBuffer;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer;
PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackARB;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
PFNGLTEXIMAGE3DPROC glTexImage3D;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
PFNGLBLENDCOLORPROC glBlendColor;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
#endif

static bool is_gl_init = false;

#ifdef DEBUG
#ifdef WIN32
#define ARB_DEBUG_OUTPUT
#endif
#endif

#ifdef ARB_DEBUG_OUTPUT
static void
#ifdef WIN32
CALLBACK
#endif
debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
              const GLchar* msg, const void *userparam)
{
    switch(source)
    {
    case GL_DEBUG_SOURCE_API_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - API");
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - WINDOW_SYSTEM");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - SHADER_COMPILER");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - THIRD_PARTY");
        break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - APPLICATION");
        break;
    case GL_DEBUG_SOURCE_OTHER_ARB:
        Log::warn("GLWrap", "OpenGL debug callback - OTHER");
        break;
    }

    switch(type)
    {
    case GL_DEBUG_TYPE_ERROR_ARB:
        Log::warn("GLWrap", "    Error type : ERROR");
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
        Log::warn("GLWrap", "    Error type : DEPRECATED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
        Log::warn("GLWrap", "    Error type : UNDEFINED_BEHAVIOR");
        break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:
        Log::warn("GLWrap", "    Error type : PORTABILITY");
        break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:
        Log::warn("GLWrap", "    Error type : PERFORMANCE");
        break;
    case GL_DEBUG_TYPE_OTHER_ARB:
        Log::warn("GLWrap", "    Error type : OTHER");
        break;
    }

    switch(severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_ARB:
        Log::warn("GLWrap", "    Severity : HIGH");
        break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:
        Log::warn("GLWrap", "    Severity : MEDIUM");
        break;
    case GL_DEBUG_SEVERITY_LOW_ARB:
        Log::warn("GLWrap", "    Severity : LOW");
        break;
    }

    if (msg)
        Log::warn("GLWrap", "    Message : %s", msg);
}
#endif

void initGL()
{
    if (is_gl_init)
        return;
    is_gl_init = true;
#ifdef _IRR_WINDOWS_API_
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
    glActiveTexture = (PFNGLACTIVETEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glActiveTexture");
    glUniform2f = (PFNGLUNIFORM2FPROC)IRR_OGL_LOAD_EXTENSION("glUniform2f");
    glUniform4i = (PFNGLUNIFORM4IPROC)IRR_OGL_LOAD_EXTENSION("glUniform4i");
    glUniform3i = (PFNGLUNIFORM3IPROC)IRR_OGL_LOAD_EXTENSION("glUniform3i");
    glUniform1i = (PFNGLUNIFORM1IPROC)IRR_OGL_LOAD_EXTENSION("glUniform1i");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramiv");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)IRR_OGL_LOAD_EXTENSION("glGetProgramInfoLog");
    glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)IRR_OGL_LOAD_EXTENSION("glTransformFeedbackVaryings");
    glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)IRR_OGL_LOAD_EXTENSION("glGetAttribLocation");
    glBlendEquation = (PFNGLBLENDEQUATIONPROC)IRR_OGL_LOAD_EXTENSION("glBlendEquation");
    glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribDivisor");
    glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)IRR_OGL_LOAD_EXTENSION("glDrawArraysInstanced");
    glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)IRR_OGL_LOAD_EXTENSION("glDrawElementsInstanced");
    glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glDeleteBuffers");
    glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)IRR_OGL_LOAD_EXTENSION("glGenVertexArrays");
    glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)IRR_OGL_LOAD_EXTENSION("glBindVertexArray");
    glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)IRR_OGL_LOAD_EXTENSION("glDeleteVertexArrays");
    glTexBuffer = (PFNGLTEXBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glTexBuffer");
    glUniform1fv = (PFNGLUNIFORM1FVPROC)IRR_OGL_LOAD_EXTENSION("glUniform1fv");
    glUniform4fv = (PFNGLUNIFORM4FVPROC)IRR_OGL_LOAD_EXTENSION("glUniform4fv");
    glBufferSubData = (PFNGLBUFFERSUBDATAPROC)IRR_OGL_LOAD_EXTENSION("glBufferSubData");
    glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)IRR_OGL_LOAD_EXTENSION("glVertexAttribIPointer");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glGenFramebuffers");
    glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)IRR_OGL_LOAD_EXTENSION("glDeleteFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glBindFramebuffer");
    glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)IRR_OGL_LOAD_EXTENSION("glFramebufferTexture2D");
    glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)IRR_OGL_LOAD_EXTENSION("glFramebufferTexture");
    glTexImage3D = (PFNGLTEXIMAGE3DPROC)IRR_OGL_LOAD_EXTENSION("glTexImage3D");
    glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)IRR_OGL_LOAD_EXTENSION("glGenerateMipmap");
    glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)IRR_OGL_LOAD_EXTENSION("glCheckFramebufferStatus");
    glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)IRR_OGL_LOAD_EXTENSION("glTexImage2DMultisample");
    glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)IRR_OGL_LOAD_EXTENSION("glBlitFramebuffer");
    glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)IRR_OGL_LOAD_EXTENSION("glGetUniformBlockIndex");
    glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)IRR_OGL_LOAD_EXTENSION("glUniformBlockBinding");
    glBlendColor = (PFNGLBLENDCOLORPROC)IRR_OGL_LOAD_EXTENSION("glBlendColor");
    glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)IRR_OGL_LOAD_EXTENSION("glCompressedTexImage2D");
    glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)IRR_OGL_LOAD_EXTENSION("glGetCompressedTexImage");
#ifdef DEBUG
    glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)IRR_OGL_LOAD_EXTENSION("glDebugMessageCallbackARB");
#endif
#endif
#ifdef ARB_DEBUG_OUTPUT
    if (glDebugMessageCallbackARB)
        glDebugMessageCallbackARB((GLDEBUGPROCARB)debugCallback, NULL);
#endif
}

// Mostly from shader tutorial
GLuint LoadShader(const char * file, unsigned type)
{
    GLuint Id = glCreateShader(type);
    char versionString[20];
    sprintf(versionString, "#version %d\n", irr_driver->getGLSLVersion());
    std::string Code = versionString;
    std::ifstream Stream(file, std::ios::in);
    Code += "//" + std::string(file) + "\n";
    if (UserConfigParams::m_ubo_disabled)
        Code += "#define UBO_DISABLED\n";
    if (irr_driver->hasVSLayerExtension())
        Code += "#define VSLayer\n";
    if (Stream.is_open())
    {
        std::string Line = "";
        while (getline(Stream, Line))
            Code += "\n" + Line;
        Stream.close();
    }
    GLint Result = GL_FALSE;
    int InfoLogLength;
    Log::info("GLWrap", "Compiling shader : %s", file);
    char const * SourcePointer = Code.c_str();
    int length = strlen(SourcePointer);
    glShaderSource(Id, 1, &SourcePointer, &length);
    glCompileShader(Id);

    glGetShaderiv(Id, GL_COMPILE_STATUS, &Result);
    if (Result == GL_FALSE)
    {
        Log::error("GLWrap", "Error in shader %s", file);
        glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetShaderInfoLog(Id, InfoLogLength, NULL, ErrorMessage);
        Log::error("GLWrap", ErrorMessage);
        delete[] ErrorMessage;
    }

    glGetError();

    return Id;
}

GLuint LoadTFBProgram(const char * vertex_file_path, const char **varyings, unsigned varyingscount)
{
    GLuint Program = glCreateProgram();
    loadAndAttach(Program, GL_VERTEX_SHADER, vertex_file_path);
    glTransformFeedbackVaryings(Program, varyingscount, varyings, GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(Program);

    GLint Result = GL_FALSE;
    int InfoLogLength;
    glGetProgramiv(Program, GL_LINK_STATUS, &Result);
    if (Result == GL_FALSE)
    {
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &InfoLogLength);
        char *ErrorMessage = new char[InfoLogLength];
        glGetProgramInfoLog(Program, InfoLogLength, NULL, ErrorMessage);
        printf(ErrorMessage);
        delete[] ErrorMessage;
    }

    glGetError();

    return Program;
}

GLuint getTextureGLuint(irr::video::ITexture *tex)
{
    return static_cast<irr::video::COpenGLTexture*>(tex)->getOpenGLTextureName();
}

GLuint getDepthTexture(irr::video::ITexture *tex)
{
    assert(tex->isRenderTarget());
    return static_cast<irr::video::COpenGLFBOTexture*>(tex)->DepthBufferTexture;
}

std::set<irr::video::ITexture *> AlreadyTransformedTexture;
void resetTextureTable()
{
    AlreadyTransformedTexture.clear();
}

void compressTexture(irr::video::ITexture *tex, bool srgb, bool premul_alpha)
{
    if (AlreadyTransformedTexture.find(tex) != AlreadyTransformedTexture.end())
        return;
    AlreadyTransformedTexture.insert(tex);

    glBindTexture(GL_TEXTURE_2D, getTextureGLuint(tex));

    std::string cached_file;
    if (UserConfigParams::m_texture_compression)
    {
        // Try to retrieve the compressed texture in cache
        std::string tex_name = irr_driver->getTextureName(tex);
        if (!tex_name.empty()) {
            cached_file = file_manager->getTextureCacheLocation(tex_name) + ".gltz";
            if (!file_manager->fileIsNewer(tex_name, cached_file)) {
                if (loadCompressedTexture(cached_file))
                    return;
            }
        }
    }

    size_t w = tex->getSize().Width, h = tex->getSize().Height;
    unsigned char *data = new unsigned char[w * h * 4];
    memcpy(data, tex->lock(), w * h * 4);
    tex->unlock();
    unsigned internalFormat, Format;
    if (tex->hasAlpha())
        Format = GL_BGRA;
    else
        Format = GL_BGR;

    if (premul_alpha)
    {
        for (unsigned i = 0; i < w * h; i++)
        {
            float alpha = data[4 * i + 3];
            if (alpha > 0.)
                alpha = pow(alpha / 255.f, 1.f / 2.2f);
            data[4 * i    ] = (unsigned char)(data[4 * i    ] * alpha);
            data[4 * i + 1] = (unsigned char)(data[4 * i + 1] * alpha);
            data[4 * i + 2] = (unsigned char)(data[4 * i + 2] * alpha);
        }
    }

    if (!UserConfigParams::m_texture_compression)
    {
        if (srgb)
            internalFormat = (tex->hasAlpha()) ? GL_SRGB_ALPHA : GL_SRGB;
        else
            internalFormat = (tex->hasAlpha()) ? GL_RGBA : GL_RGB;
    }
    else
    {
        if (srgb)
            internalFormat = (tex->hasAlpha()) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
        else
            internalFormat = (tex->hasAlpha()) ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, Format, GL_UNSIGNED_BYTE, (GLvoid *)data);
    glGenerateMipmap(GL_TEXTURE_2D);
    delete[] data;

    if (UserConfigParams::m_texture_compression && !cached_file.empty())
    {
        // Save the compressed texture in the cache for later use.
        saveCompressedTexture(cached_file);
    }
}

//-----------------------------------------------------------------------------
/** Try to load a compressed texture from the given file name.
 *   Data in the specified file need to have a specific format. See the 
 *   saveCompressedTexture() function for a description of the format.
 *   \return true if the loading succeeded, false otherwise.
 *   \see saveCompressedTexture
 */
bool loadCompressedTexture(const std::string& compressed_tex)
{
    std::ifstream ifs(compressed_tex.c_str(), std::ios::in | std::ios::binary);
    if (!ifs.is_open())
        return false;

    int internal_format;
    int w, h;
    int size = -1;
    ifs.read((char*)&internal_format, sizeof(int));
    ifs.read((char*)&w, sizeof(int));
    ifs.read((char*)&h, sizeof(int));
    ifs.read((char*)&size, sizeof(int));

    if (ifs.fail() || size == -1)
        return false;

    char *data = new char[size];
    ifs.read(data, size);
    if (!ifs.fail())
    {
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, internal_format,
                               w, h, 0, size, (GLvoid*)data);
        glGenerateMipmap(GL_TEXTURE_2D);
        delete[] data;
        ifs.close();
        return true;
    }
    delete[] data;
    return false;
}

//-----------------------------------------------------------------------------
/** Try to save the last texture sent to glTexImage2D in a file of the given
 *   file name. This function should only be used for textures sent to
 *   glTexImage2D with a compressed internal format as argument.<br>
 *   \note The following format is used to save the compressed texture:<br>
 *         <internal-format><width><height><size><data> <br>
 *         The first four elements are integers and the last one is stored
 *         on \c size bytes.
 *   \see loadCompressedTexture
 */
void saveCompressedTexture(const std::string& compressed_tex)
{
    int internal_format, width, height, size, compressionSuccessful;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, (GLint *)&internal_format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, (GLint *)&width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, (GLint *)&height);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, (GLint *)&compressionSuccessful);
    if (!compressionSuccessful)
        return;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, (GLint *)&size);

    char *data = new char[size];
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, (GLvoid*)data);
    std::ofstream ofs(compressed_tex.c_str(), std::ios::out | std::ios::binary);
    if (ofs.is_open())
    {
        ofs.write((char*)&internal_format, sizeof(int));
        ofs.write((char*)&width, sizeof(int));
        ofs.write((char*)&height, sizeof(int));
        ofs.write((char*)&size, sizeof(int));
        ofs.write(data, size);
        ofs.close();
    }
    delete[] data;
}

void setTexture(unsigned TextureUnit, GLuint TextureId, GLenum MagFilter, GLenum MinFilter, bool allowAF)
{
    glActiveTexture(GL_TEXTURE0 + TextureUnit);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int aniso = UserConfigParams::m_anisotropic;
    if (aniso == 0) aniso = 1;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, allowAF ? (float)aniso : 1.0f);

    glGetError();
}

ScopedGPUTimer::ScopedGPUTimer(GPUTimer &timer)
{
    if (!UserConfigParams::m_profiler_enabled) return;
    if (profiler.isFrozen()) return;

#ifdef GL_TIME_ELAPSED
    irr::video::COpenGLDriver *gl_driver = (irr::video::COpenGLDriver *)irr_driver->getDevice()->getVideoDriver();
    if (!timer.initialised)
    {
        gl_driver->extGlGenQueries(1, &timer.query);
        timer.initialised = true;
    }
    gl_driver->extGlBeginQuery(GL_TIME_ELAPSED, timer.query);
#endif
}
ScopedGPUTimer::~ScopedGPUTimer()
{
    if (!UserConfigParams::m_profiler_enabled) return;
    if (profiler.isFrozen()) return;
    
#ifdef GL_TIME_ELAPSED
    irr::video::COpenGLDriver *gl_driver = (irr::video::COpenGLDriver *)irr_driver->getDevice()->getVideoDriver();
    gl_driver->extGlEndQuery(GL_TIME_ELAPSED);
#endif
}

GPUTimer::GPUTimer() : initialised(false)
{
}

unsigned GPUTimer::elapsedTimeus()
{
    if (!initialised)
        return 0;
    GLuint result;
    irr::video::COpenGLDriver *gl_driver = (irr::video::COpenGLDriver *)irr_driver->getDevice()->getVideoDriver();
    gl_driver->extGlGetQueryObjectuiv(query, GL_QUERY_RESULT, &result);
    return result / 1000;
}

FrameBuffer::FrameBuffer() {}

FrameBuffer::FrameBuffer(const std::vector<GLuint> &RTTs, size_t w, size_t h, bool layered) :
    DepthTexture(0), RenderTargets(RTTs), width(w), height(h)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (layered)
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, RTTs[i], 0);
    }
    else
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, RTTs[i], 0);
    }
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
}

FrameBuffer::FrameBuffer(const std::vector<GLuint> &RTTs, GLuint DS, size_t w, size_t h, bool layered) :
    DepthTexture(DS), RenderTargets(RTTs), width(w), height(h)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    if (layered)
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, RTTs[i], 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, DS, 0);
    }
    else
    {
        for (unsigned i = 0; i < RTTs.size(); i++)
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, RTTs[i], 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DS, 0);
    }
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    assert(result == GL_FRAMEBUFFER_COMPLETE_EXT);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
    irr::video::COpenGLDriver *gl_driver = (irr::video::COpenGLDriver*)irr_driver->getDevice()->getVideoDriver();
    GLenum bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    gl_driver->extGlDrawBuffers(RenderTargets.size(), bufs);
}

void FrameBuffer::Blit(const FrameBuffer &Src, FrameBuffer &Dst, GLbitfield mask, GLenum filter)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, Src.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Dst.fbo);
    glBlitFramebuffer(0, 0, Src.width, Src.height, 0, 0, Dst.width, Dst.height, mask, filter);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void FrameBuffer::BlitToDefault(size_t x0, size_t y0, size_t x1, size_t y1)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void draw3DLine(const core::vector3df& start,
                const core::vector3df& end, irr::video::SColor color)
{
    if (!irr_driver->isGLSL()) {
        irr_driver->getVideoDriver()->draw3DLine(start, end, color);
        return;
    }

    float vertex[6] = {
        start.X, start.Y, start.Z,
        end.X, end.Y, end.Z
    };

    glBindVertexArray(UtilShader::ColoredLine::vao);
    glBindBuffer(GL_ARRAY_BUFFER, UtilShader::ColoredLine::vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(float), vertex);

    glUseProgram(UtilShader::ColoredLine::Program);
    UtilShader::ColoredLine::setUniforms(color);
    glDrawArrays(GL_LINES, 0, 2);

    glGetError();
}

static void drawTexColoredQuad(const video::ITexture *texture, const video::SColor *col, float width, float height,
                               float center_pos_x, float center_pos_y, float tex_center_pos_x, float tex_center_pos_y,
                               float tex_width, float tex_height)
{
    unsigned colors[] = {
        col[0].getRed(), col[0].getGreen(), col[0].getBlue(), col[0].getAlpha(),
        col[1].getRed(), col[1].getGreen(), col[1].getBlue(), col[1].getAlpha(),
        col[2].getRed(), col[2].getGreen(), col[2].getBlue(), col[2].getAlpha(),
        col[3].getRed(), col[3].getGreen(), col[3].getBlue(), col[3].getAlpha(),
    };

    glBindBuffer(GL_ARRAY_BUFFER, UIShader::ColoredTextureRectShader::colorvbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 16 * sizeof(unsigned), colors);

    glUseProgram(UIShader::ColoredTextureRectShader::Program);
    glBindVertexArray(UIShader::ColoredTextureRectShader::vao);

    setTexture(0, static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName(), GL_LINEAR, GL_LINEAR);
    UIShader::TextureRectShader::setUniforms(center_pos_x, center_pos_y, width, height, tex_center_pos_x, tex_center_pos_y, tex_width, tex_height, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}

void drawTexQuad(const video::ITexture *texture, float width, float height,
                 float center_pos_x, float center_pos_y, float tex_center_pos_x, float tex_center_pos_y,
                 float tex_width, float tex_height)
{
    glUseProgram(UIShader::TextureRectShader::Program);
    glBindVertexArray(UIShader::TextureRectShader::vao);

    setTexture(0, static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName(), GL_LINEAR, GL_LINEAR);
    UIShader::TextureRectShader::setUniforms(center_pos_x, center_pos_y, width, height, tex_center_pos_x, tex_center_pos_y, tex_width, tex_height, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGetError();
}

static void
getSize(const video::ITexture* texture, const core::rect<s32>& destRect,
        const core::rect<s32>& sourceRect,
        float &width, float &height,
        float &center_pos_x, float &center_pos_y,
        float &tex_width, float &tex_height,
        float &tex_center_pos_x, float &tex_center_pos_y
    )
{
    core::dimension2d<u32> frame_size =
        irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
    const int screen_w = frame_size.Width;
    const int screen_h = frame_size.Height;
    center_pos_x = float(destRect.UpperLeftCorner.X + destRect.LowerRightCorner.X);
    center_pos_x /= screen_w;
    center_pos_x -= 1.;
    center_pos_y = float(destRect.UpperLeftCorner.Y + destRect.LowerRightCorner.Y);
    center_pos_y /= screen_h;
    center_pos_y = float(1.f - center_pos_y);
    width = float(destRect.LowerRightCorner.X - destRect.UpperLeftCorner.X);
    width /= screen_w;
    height = float(destRect.LowerRightCorner.Y - destRect.UpperLeftCorner.Y);
    height /= screen_h;

    const core::dimension2d<u32>& ss = texture->getOriginalSize();
    tex_center_pos_x = float(sourceRect.UpperLeftCorner.X + sourceRect.LowerRightCorner.X);
    tex_center_pos_x /= ss.Width * 2.f;
    tex_center_pos_y = float(sourceRect.UpperLeftCorner.Y + sourceRect.LowerRightCorner.Y);
    tex_center_pos_y /= ss.Height * 2.f;
    tex_width = float(sourceRect.LowerRightCorner.X - sourceRect.UpperLeftCorner.X);
    tex_width /= ss.Width * 2.f;
    tex_height = float(sourceRect.LowerRightCorner.Y - sourceRect.UpperLeftCorner.Y);
    tex_height /= ss.Height * 2.f;

    if (texture->isRenderTarget())
    {
        tex_height = -tex_height;
    }

    const f32 invW = 1.f / static_cast<f32>(ss.Width);
    const f32 invH = 1.f / static_cast<f32>(ss.Height);
    const core::rect<f32> tcoords(
        sourceRect.UpperLeftCorner.X * invW,
        sourceRect.UpperLeftCorner.Y * invH,
        sourceRect.LowerRightCorner.X * invW,
        sourceRect.LowerRightCorner.Y *invH);
}

void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
                 const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
                 const video::SColor &colors, bool useAlphaChannelOfTexture)
{
    if (!irr_driver->isGLSL()) {
        video::SColor duplicatedArray[4] = {
            colors, colors, colors, colors
        };
        draw2DImage(texture, destRect, sourceRect, clipRect, duplicatedArray, useAlphaChannelOfTexture);
        return;
    }

    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize(texture, destRect, sourceRect, width, height, center_pos_x, center_pos_y,
            tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clipRect)
    {
        if (!clipRect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
                  clipRect->getWidth(), clipRect->getHeight());
    }

    glUseProgram(UIShader::UniformColoredTextureRectShader::Program);
    glBindVertexArray(UIShader::UniformColoredTextureRectShader::vao);

    setTexture(0, static_cast<const irr::video::COpenGLTexture*>(texture)->getOpenGLTextureName(), GL_LINEAR, GL_LINEAR);
    UIShader::UniformColoredTextureRectShader::setUniforms(center_pos_x, center_pos_y, width, height, tex_center_pos_x, tex_center_pos_y, tex_width, tex_height,colors, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (clipRect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
                 const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect,
                 const video::SColor* const colors, bool useAlphaChannelOfTexture)
{
    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DImage(texture, destRect, sourceRect, clipRect, colors, useAlphaChannelOfTexture);
        return;
    }

    float width, height,
        center_pos_x, center_pos_y,
        tex_width, tex_height,
        tex_center_pos_x, tex_center_pos_y;

    getSize(texture, destRect, sourceRect, width, height, center_pos_x, center_pos_y,
            tex_width, tex_height, tex_center_pos_x, tex_center_pos_y);

    if (useAlphaChannelOfTexture)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    if (clipRect)
    {
        if (!clipRect->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clipRect->UpperLeftCorner.X, renderTargetSize.Height - clipRect->LowerRightCorner.Y,
                  clipRect->getWidth(), clipRect->getHeight());
    }
    if (colors)
        drawTexColoredQuad(texture, colors, width, height, center_pos_x, center_pos_y,
                           tex_center_pos_x, tex_center_pos_y, tex_width, tex_height);
    else
        drawTexQuad(texture, width, height, center_pos_x, center_pos_y,
                    tex_center_pos_x, tex_center_pos_y, tex_width, tex_height);
    if (clipRect)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}

void GL32_draw2DRectangle(video::SColor color, const core::rect<s32>& position,
                          const core::rect<s32>* clip)
{

    if (!irr_driver->isGLSL())
    {
        irr_driver->getVideoDriver()->draw2DRectangle(color, position, clip);
        return;
    }

    core::dimension2d<u32> frame_size =
        irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
    const int screen_w = frame_size.Width;
    const int screen_h = frame_size.Height;
    float center_pos_x = float(position.UpperLeftCorner.X + position.LowerRightCorner.X);
    center_pos_x /= screen_w;
    center_pos_x -= 1;
    float center_pos_y = float(position.UpperLeftCorner.Y + position.LowerRightCorner.Y);
    center_pos_y /= screen_h;
    center_pos_y = 1 - center_pos_y;
    float width = float(position.LowerRightCorner.X - position.UpperLeftCorner.X);
    width /= screen_w;
    float height = float(position.LowerRightCorner.Y - position.UpperLeftCorner.Y);
    height /= screen_h;

    if (color.getAlpha() < 255)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    if (clip)
    {
        if (!clip->isValid())
            return;

        glEnable(GL_SCISSOR_TEST);
        const core::dimension2d<u32>& renderTargetSize = irr_driver->getVideoDriver()->getCurrentRenderTargetSize();
        glScissor(clip->UpperLeftCorner.X, renderTargetSize.Height - clip->LowerRightCorner.Y,
                  clip->getWidth(), clip->getHeight());
    }

    glUseProgram(UIShader::ColoredRectShader::Program);
    glBindVertexArray(UIShader::ColoredRectShader::vao);
    UIShader::ColoredRectShader::setUniforms(center_pos_x, center_pos_y, width, height, color);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    if (clip)
        glDisable(GL_SCISSOR_TEST);
    glUseProgram(0);

    glGetError();
}
