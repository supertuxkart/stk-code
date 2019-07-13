// Copyright (C) 2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in Irrlicht.h

#ifndef __C_EAGL_MANAGER_H_INCLUDED__
#define __C_EAGL_MANAGER_H_INCLUDED__

#include "IrrCompileConfig.h"

#include "SIrrCreationParameters.h"
#include "SExposedVideoData.h"
#include "IContextManager.h"

namespace irr
{
namespace video
{
    // EAGL manager.
    class CEAGLManager : public IContextManager
    {
    public:
        //! Constructor.
        CEAGLManager();

        //! Destructor.
        virtual ~CEAGLManager();

        // Initialize EAGL.
        /* This method checks if a view has CAEAGLLayer and grabs it if it does, anyway surface and context
        aren't create. */
        bool initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data);

        // Terminate EAGL.
        /* Terminate EAGL context. This method break both existed surface and context. */
        void terminate();

        // Create EAGL surface.
        /* This method configure CAEAGLLayer. */
        bool generateSurface();

        // Destroy EAGL surface.
        /* This method reset CAEAGLLayer states. */
        void destroySurface();

        // Create EAGL context.
        /* This method create and activate EAGL context. */
        bool generateContext();

        // Destroy EAGL context.
        /* This method destroy EAGL context. */
        void destroyContext();

        const SExposedVideoData& getContext() const;

        bool activateContext(const SExposedVideoData& videoData);

        // Swap buffers.
        bool swapBuffers();

    private:
        SIrrlichtCreationParameters Params;
        SExposedVideoData Data;

        bool Configured;

        void* DataStorage;

        struct SFrameBuffer
        {
            SFrameBuffer() : BufferID(0), ColorBuffer(0), DepthBuffer(0)
            {
            }

            u32 BufferID;
            u32 ColorBuffer;
            u32 DepthBuffer;
        };

        SFrameBuffer FrameBuffer;
    };
}
}

#endif
