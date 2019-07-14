// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#include "CEAGLManager.h"

#include "irrString.h"
#include "os.h"

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

namespace irr
{
namespace video
{

struct SEAGLManagerDataStorage
{
    SEAGLManagerDataStorage() : Layer(0), Context(0)
    {
    }

    CAEAGLLayer* Layer;
    EAGLContext* Context;
};

CEAGLManager::CEAGLManager() : IContextManager(), Configured(false), DataStorage(0)
{
#ifdef _DEBUG
    setDebugName("CEAGLManager");
#endif

    DataStorage = new SEAGLManagerDataStorage();
}

CEAGLManager::~CEAGLManager()
{
    destroyContext();
    destroySurface();
    terminate();

    delete static_cast<SEAGLManagerDataStorage*>(DataStorage);
}

bool CEAGLManager::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data)
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);

    if (dataStorage->Layer != nil)
        return true;

    Params = params;
    Data = data;

    UIView* view = (__bridge UIView*)data.OpenGLiOS.View;

    if (view == nil || ![[view layer] isKindOfClass:[CAEAGLLayer class]])
    {
        os::Printer::log("Could not get EAGL display.");
        return false;
    }

    dataStorage->Layer = (CAEAGLLayer*)[view layer];
    dataStorage->Layer.contentsScale = view.contentScaleFactor;

    return true;
}

void CEAGLManager::terminate()
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);

    [EAGLContext setCurrentContext:0];

    destroySurface();

    if (dataStorage->Layer != nil)
        dataStorage->Layer = 0;
}

bool CEAGLManager::generateSurface()
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);
    CAEAGLLayer* layer = dataStorage->Layer;

    if (layer == nil)
        return false;

    if (Configured)
        return true;

    NSDictionary* attribs = [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithBool:NO],
        kEAGLDrawablePropertyRetainedBacking,
        (Params.Bits > 16) ? kEAGLColorFormatRGBA8 : kEAGLColorFormatRGB565,
        kEAGLDrawablePropertyColorFormat,
        nil];

    [layer setOpaque:(Params.WithAlphaChannel) ? YES : NO];
    [layer setDrawableProperties:attribs];

    Configured = true;

    return true;
}

void CEAGLManager::destroySurface()
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);
    CAEAGLLayer* layer = dataStorage->Layer;

    if (layer == nil)
        return;

    [layer setOpaque:NO];
    [layer setDrawableProperties:nil];

    Configured = false;
}

bool CEAGLManager::generateContext()
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);

    if (dataStorage->Context != nil || !Configured)
        return false;

    EAGLRenderingAPI OpenGLESVersion = kEAGLRenderingAPIOpenGLES2;

    switch (Params.DriverType)
    {
    case EDT_OGLES2:
    {
        // For IOS we use 64bit only and all 64bit ios devices support GLES3 anyway
        if (!Params.ForceLegacyDevice)
            OpenGLESVersion = kEAGLRenderingAPIOpenGLES3;
        else
            OpenGLESVersion = kEAGLRenderingAPIOpenGLES2;
        break;
    }
    default:
        break;
    }

    dataStorage->Context = [[EAGLContext alloc] initWithAPI:OpenGLESVersion];

    if (dataStorage->Context == nil)
    {
        os::Printer::log("Could not create EAGL context.", ELL_ERROR);
        return false;
    }

    Data.OpenGLiOS.Context = (__bridge void*)dataStorage->Context;

    os::Printer::log("EAGL context created with OpenGLESVersion: ", core::stringc(static_cast<int>(OpenGLESVersion)), ELL_DEBUG);

    return true;
}

void CEAGLManager::destroyContext()
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);

    [dataStorage->Context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:nil];

    if (FrameBuffer.BufferID != 0)
    {
        glDeleteFramebuffersOES(1, &FrameBuffer.BufferID);
        FrameBuffer.BufferID = 0;
    }

    if (FrameBuffer.ColorBuffer != 0)
    {
        glDeleteRenderbuffersOES(1, &FrameBuffer.ColorBuffer);
        FrameBuffer.ColorBuffer = 0;
    }

    if (FrameBuffer.DepthBuffer != 0)
    {
        glDeleteRenderbuffersOES(1, &FrameBuffer.DepthBuffer);
        FrameBuffer.DepthBuffer = 0;
    }

    [EAGLContext setCurrentContext:0];

    if (dataStorage->Context != nil)
        dataStorage->Context = 0;

    Data.OpenGLiOS.Context = 0;
}

bool CEAGLManager::activateContext(const SExposedVideoData& videoData)
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);
    EAGLContext* context = dataStorage->Context;

    bool status = false;

    if (context != nil)
    {
        status = ([EAGLContext currentContext] == context || [EAGLContext setCurrentContext:context]);
    }

    if (status)
    {
        if (FrameBuffer.ColorBuffer == 0)
        {
            glGenRenderbuffersOES(1, &FrameBuffer.ColorBuffer);
            glBindRenderbufferOES(GL_RENDERBUFFER_OES, FrameBuffer.ColorBuffer);
            [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:dataStorage->Layer];
        }

        if (FrameBuffer.DepthBuffer == 0)
        {
            GLenum depth = (Params.ZBufferBits >= 24) ? GL_DEPTH_COMPONENT24_OES : GL_DEPTH_COMPONENT16_OES;

            glGenRenderbuffersOES(1, &FrameBuffer.DepthBuffer);
            glBindRenderbufferOES(GL_RENDERBUFFER_OES, FrameBuffer.DepthBuffer);
            glRenderbufferStorageOES(GL_RENDERBUFFER_OES, depth, Params.WindowSize.Width, Params.WindowSize.Height);
        }

        if (FrameBuffer.BufferID == 0)
        {
            glGenFramebuffersOES(1, &FrameBuffer.BufferID);
            glBindFramebufferOES(GL_FRAMEBUFFER_OES, FrameBuffer.BufferID);
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, FrameBuffer.ColorBuffer);
            glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, FrameBuffer.DepthBuffer);
        }

        glBindFramebufferOES(GL_FRAMEBUFFER_OES, FrameBuffer.BufferID);
    }
    else
    {
        os::Printer::log("Could not make EGL context current.");
    }

    return status;
}

const SExposedVideoData& CEAGLManager::getContext() const
{
    return Data;
}

bool CEAGLManager::swapBuffers()
{
    SEAGLManagerDataStorage* dataStorage = static_cast<SEAGLManagerDataStorage*>(DataStorage);
    EAGLContext* context = dataStorage->Context;

    bool status = false;

    if (context != nil && context == [EAGLContext currentContext])
    {
        glBindRenderbufferOES(GL_RENDERBUFFER_OES, FrameBuffer.ColorBuffer);
        [context presentRenderbuffer:GL_RENDERBUFFER_OES];

        status = true;
    }

    return status;
}

}
}
