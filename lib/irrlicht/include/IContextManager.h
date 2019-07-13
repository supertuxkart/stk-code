// Copyright (C) 2013-2015 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_I_CONTEXT_MANAGER_H_INCLUDED__
#define __IRR_I_CONTEXT_MANAGER_H_INCLUDED__

#include "SExposedVideoData.h"
#include "SIrrCreationParameters.h"

namespace irr
{
namespace video
{
    // For system specific window contexts (used for OpenGL)
    class IContextManager : public virtual IReferenceCounted
    {
    public:
        //! Initialize manager with device creation parameters and device window (passed as exposed video data)
        virtual bool initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data) =0;

        //! Terminate manager, any cleanup that is left over. Manager needs a new initialize to be usable again
        virtual void terminate() =0;

        //! Create surface based on current window set
        virtual bool generateSurface() =0;

        //! Destroy current surface
        virtual void destroySurface() =0;

        //! Create context based on current surface
        virtual bool generateContext() =0;

        //! Destroy current context
        virtual void destroyContext() =0;

        //! Get current context
        virtual const SExposedVideoData& getContext() const =0;

        //! Change render context, disable old and activate new defined by videoData
        virtual bool activateContext(const SExposedVideoData& videoData) =0;

        //! Swap buffers.
        virtual bool swapBuffers() =0;
    };

} // end namespace video
} // end namespace irr


#endif
