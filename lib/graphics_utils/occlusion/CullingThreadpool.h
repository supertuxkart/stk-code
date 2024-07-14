////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////
#pragma once

 /*!
  * \file CullingThreadpool.h
  * \brief Worker threadpool example for threaded masked occlusion culling.
  *
  * This class implements a threadpool for occluder rendering. Calls to CullingThreadpool::RenderTriangle()
  * will immediately return, after adding work items to a queue, and occluder rendering is performed
  * by worker threads as quickly as possible. Occlusion queries are performed directly on the calling 
  * threadand can be performed either synchronosly, by calling Flush() before executing the query, or 
  * asynchronosly, by performing the query without waiting for the worker threads to finish. 
  *
  * Note that this implementation should be considered an example rather than the best threading
  * solution. You may want to integrate threading in your own task system, and it may also be beneficial 
  * to thread the traversal code. Refer to MaskedOcclusionCulling::BinTriangles() and 
  * MaskedOcclusionCulling::RenderTrilist() for functions that can be used to make your own 
  * threaded culling system. 
  */

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "MaskedOcclusionCulling.h"

class CullingThreadpool
{
protected:
	static const int TRIS_PER_JOB = 1024; // Maximum number of triangles per job (bigger drawcalls are split), affects memory requirements

	typedef MaskedOcclusionCulling::CullingResult   CullingResult;
	typedef MaskedOcclusionCulling::ClipPlanes      ClipPlanes;
	typedef MaskedOcclusionCulling::BackfaceWinding BackfaceWinding;
	typedef MaskedOcclusionCulling::ScissorRect     ScissorRect;
	typedef MaskedOcclusionCulling::VertexLayout    VertexLayout;
	typedef MaskedOcclusionCulling::TriList         TriList;

	// Small utility class for 4x4 matrices
	struct Matrix4x4
	{
		float mValues[16];
		Matrix4x4() {}
		Matrix4x4(const float *matrix)
		{
			for (int i = 0; i < 16; ++i)
				mValues[i] = matrix[i];
		}
	};

	// Internal utility class for a (mostly) lockless queue for binning & rendering jobs
	struct RenderJobQueue
	{
		struct BinningJob
		{
			const float*        mVerts;
			const unsigned int* mTris;
			unsigned int        nTris;

			const float*        mMatrix;
			ClipPlanes          mClipPlanes;
			BackfaceWinding     mBfWinding;
			const VertexLayout* mVtxLayout;
		};

		struct Job
		{
			volatile unsigned int mBinningJobStartedIdx;
			volatile unsigned int mBinningJobCompletedIdx;
			BinningJob            mBinningJob;
			TriList               *mRenderJobs;
		};

		unsigned int          mNumBins;
		unsigned int          mMaxJobs;

		volatile unsigned int mWritePtr;
		std::atomic_uint      mBinningPtr;
		std::atomic_uint      *mRenderPtrs;
		std::atomic_uint      *mBinMutexes;

		float                 *mTrilistData;
		Job                   *mJobs;

		RenderJobQueue(unsigned int nBins, unsigned int maxJobs);
		~RenderJobQueue();

		unsigned int GetMinRenderPtr() const;
		unsigned int GetBestGlobalQueue() const;
		bool IsPipelineEmpty() const;

		bool CanWrite() const;
		bool CanBin() const;

		Job *GetWriteJob();
		void AdvanceWriteJob();

		Job *GetBinningJob();
		void FinishedBinningJob(Job *job);

		Job *GetRenderJob(int binIdx);
		void AdvanceRenderJob(int binIdx);

		void Reset();
	};

	// Internal utility class for state (matrix / vertex layout)
	template<class T> struct StateData
	{
		unsigned int mMaxJobs;
		unsigned int mCurrentIdx;
		T            *mData;

		StateData(unsigned int maxJobs);
		~StateData();
		void AddData(const T &data);
		const T *GetData() const;
	};

	// Number of worker threads and bins
	unsigned int            mNumThreads;
	unsigned int            mNumBins;
	unsigned int            mMaxJobs;
	unsigned int            mBinsW;
	unsigned int            mBinsH;

	// Threads and control variables
	std::mutex              mSuspendedMutex;
	std::condition_variable mSuspendedCV;
	volatile bool           mKillThreads;
	volatile bool           mSuspendThreads;
	volatile unsigned int   mNumSuspendedThreads;
	std::thread             *mThreads;

	// State variables and command queue
	const float             *mCurrentMatrix;
	StateData<Matrix4x4>    mModelToClipMatrices;
	StateData<VertexLayout> mVertexLayouts;
	RenderJobQueue          *mRenderQueue;

	// Occlusion culling object and related scissor rectangles
	ScissorRect             *mRects;
	MaskedOcclusionCulling  *mMOC;

	void SetupScissors();

	static void ThreadRun(CullingThreadpool *threadPool, unsigned int threadId);
	void ThreadMain(unsigned int threadIdx);

public:
	/*!
	 * \brief Creates a new threadpool for masked occlusion culling. This object has a 
	 *        similar API to the MaskedOcclusionCulling class, but performs occluder
	 *        rendering asynchronously on worker threads (similar to how DX/GL works). 
	 *
	 * \param numThreads Number of worker threads to perform occluder rendering. Best 
	 *        balance may be scene/machine dependent, but it's good practice to leave at 
	 *        least one full core (2 threads with hyperthreading) for the main thread.
	 * \param binsW The screen is divided into binsW x binsH rectangular bins for load
	 *        balancing. The number of bins should be atleast equal to the number of
	 *        worker threads.
	 * \param binsH See description for the binsW parameter.
	 * \param maxJobs Maximum number of jobs that may be in flight at any given time. If
	 *        the caller thread generates jobs faster than the worker threads can finish
	 *        them, then the job queue will fill up and the caller thread will stall once
	 *        "maxJobs" items have been queued up. For culling systems interleaving occlusion 
	 *        queries and rendering, this value should be kept quite low to minimize false
	 *        positives (see TestRect()). We've observed that 32 [default] items typically
	 *        works well for our interleaved queries, while also allowing good load-balancing,
	 *        and this is the recommended setting. 
	 */
	CullingThreadpool(unsigned int numThreads, unsigned int binsW, unsigned int binsH, unsigned int maxJobs = 32);
	
	/*!
	 * \brief Destroys the threadpool and terminates all worker threads.
	 */
	~CullingThreadpool();

	/*!
	 * \brief Wakes up culling worker threads from suspended sleep, and puts them in a
	 *        ready state (using an idle spinlock with significantly higher CPU overhead). 
	 *
	 * It may take on the order of 100us to wake up the threads, so this function should
	 * preferably be called slightly ahead of starting occlusion culling work.
	 */
	void WakeThreads();

	/*!
	 * \brief Suspend all culling worker threads to a low CPU overhead sleep state. 
	 *
	 * For performance and latency reasons, the culling work is performed in an active 
	 * processing loop (with no thread sleeping) with high CPU overhead. In a system 
	 * with more worker threads it's important to put the culling worker threads in a 
	 * low overhead sleep state after occlusion culling work has completed.
	 */
	void SuspendThreads();

	/*!
	 * \brief Waits for all outstanding occluder rendering work to complete. Can be used
	 *        to ensure that rendering has completed before performing a TestRect() or 
	 *        TestTriangles() call.
	 */
	void Flush();

	/*
	 * \brief Sets the MaskedOcclusionCulling object (buffer) to be used for rendering and
	 *        testing calls. This method causes a Flush() to ensure that all unfinished 
	 *        rendering is completed.
	 */
	void SetBuffer(MaskedOcclusionCulling *moc);

	/*
	 * \brief Changes the resolution of the occlusion buffer, see MaskedOcclusionCulling::SetResolution().
	 *        This method causes a Flush() to ensure that all unfinished rendering is completed.
	 */
	void SetResolution(unsigned int width, unsigned int height);

	/*
	 * \brief Sets the near clipping plane, see MaskedOcclusionCulling::SetNearClipPlane(). This 
	 *        method causes a Flush() to ensure that all unfinished rendering is completed.
	 */
	void SetNearClipPlane(float nearDist);

	/*
	 * \brief Sets the model to clipspace transform matrix used for the RenderTriangles() and TestTriangles() 
	 *        function calls. The contents of the matrix is copied, and it's safe to modify it without calling
	 *        Flush(). The copy may be costly, which is the reason for passing this parameter as "state".
	 *
	 * \param modelToClipMatrix All vertices will be transformed by the specified model to clipspace matrix. 
	 *        Passing nullptr [default] disables the transform (equivalent to using an identity matrix).
	 */
	void SetMatrix(const float *modelToClipMatrix = nullptr);

	/*
	 * \brief Sets the vertex layout used for the RenderTriangles() and TestTriangles() function calls.
	 *        The vertex layout is copied, and it's safe to modify it without calling Flush(). The copy 
	 *        may be costly, which is the reason for passing this parameter as "state".
	 *
	 * \param vtxLayout A struct specifying the vertex layout (see struct for detailed
	 *        description). For best performance, it is advicable to store position data
	 *        as compactly in memory as possible.
	 */
	void SetVertexLayout(const VertexLayout &vtxLayout = VertexLayout(16, 4, 12));

	/*
	 * \brief Clears the occlusion buffer, see MaskedOcclusionCulling::ClearBuffer(). This method
	 *        causes a Flush() to ensure that all unfinished rendering is completed.
	 */
	void ClearBuffer();

	/*
	 * \brief Asynchronously render occluder triangles, see MaskedOcclusionCulling::RenderTriangles().
	 *
	 * This method puts the drawcall into a command queue, and immediately returns. The rendering is 
	 * performed by the worker threads at the earliest opportunity.
	 *
	 * <B>Important:</B> As rendering is performed asynchronously, the application is not allowed to 
	 * change the contents of the *inVtx or *inTris buffers until after rendering is completed. If 
	 * you wish to use dynamic buffers, the application must perform a Flush() to ensure that rendering 
	 * is finished, or make sure to rotate between more buffers than the maximum number of outstanding
	 * render jobs (see the CullingThreadpool() constructor).
	 */
	void RenderTriangles(const float *inVtx, const unsigned int *inTris, int nTris, BackfaceWinding bfWinding = MaskedOcclusionCulling::BACKFACE_CW, ClipPlanes clipPlaneMask = MaskedOcclusionCulling::CLIP_PLANE_ALL);

	/*
	 * \brief Occlusion query for a rectangle with a given depth, see MaskedOcclusionCulling::TestRect().
	 *
	 * <B>Important:</B> This method is performed on the main thread and does not wait for outstanding 
	 * occluder rendering to be finished. To ensure that all occluder rendering is completed you must 
	 * perform a Flush() prior to calling this function. 
	 *
	 * It is conservatively correct to perform occlusion queries without calling Flush() (it may only 
	 * lead to objects being incorrectly classified as visible), and it can lead to much better performance 
	 * if occlusion queries are used for traversing a BVH or similar data structure. It's possible to 
	 * use "asynchronous" queries during traversal, and removing false positives later, when rendering 
	 * has completed.
	 */
	CullingResult TestRect(float xmin, float ymin, float xmax, float ymax, float wmin);

	/*
	 * \brief Occlusion query for a mesh, see MaskedOcclusionCulling::TestTriangles().
	 *
	 * <B>Important:</B> See the TestRect() method for a brief discussion about asynchronous occlusion 
	 * queries.
	 */
	CullingResult TestTriangles(const float *inVtx, const unsigned int *inTris, int nTris, BackfaceWinding bfWinding = MaskedOcclusionCulling::BACKFACE_CW, ClipPlanes clipPlaneMask = MaskedOcclusionCulling::CLIP_PLANE_ALL);

	/*!
	 * \brief Creates a per-pixel depth buffer from the hierarchical z buffer representation, see
	 *        MaskedOcclusionCulling::ComputePixelDepthBuffer(). This method causes a Flush() to 
	 *        ensure that all unfinished rendering is completed.
	 */
	void ComputePixelDepthBuffer(float *depthData, bool flipY);
};
