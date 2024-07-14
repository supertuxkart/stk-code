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
#include <assert.h>
#include "CullingThreadpool.h"

#define SAFE_DELETE(X) {if (X != nullptr) delete X; X = nullptr;}
#define SAFE_DELETE_ARRAY(X) {if (X != nullptr) delete[] X; X = nullptr;}

template<class T> CullingThreadpool::StateData<T>::StateData(unsigned int maxJobs) :
	mMaxJobs(maxJobs),
	mCurrentIdx(~0)
{
	mData = new T[mMaxJobs];
}

template<class T> CullingThreadpool::StateData<T>::~StateData() 
{
	SAFE_DELETE_ARRAY(mData);
}

template<class T> void CullingThreadpool::StateData<T>::AddData(const T &data) 
{ 
	mCurrentIdx++; mData[mCurrentIdx % mMaxJobs] = data; 
}

template<class T> const T *CullingThreadpool::StateData<T>::GetData() const
{ 
	return &mData[mCurrentIdx % mMaxJobs];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper class: Mostly lockless queue for render jobs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CullingThreadpool::RenderJobQueue::RenderJobQueue(unsigned int nBins, unsigned int maxJobs) : 
	mNumBins(nBins),
	mMaxJobs(maxJobs)
{
	mRenderPtrs = new std::atomic_uint[mNumBins];
	mBinMutexes = new std::atomic_uint[mNumBins];
	for (unsigned int i = 0; i < mNumBins; ++i)
		mBinMutexes[i] = 0;

	mJobs = new Job[mMaxJobs];
	for (unsigned int i = 0; i < mMaxJobs; ++i)
		mJobs[i].mRenderJobs = new TriList[mNumBins];

	// Compute worst case job size (we allocate memory for the worst case)
	const unsigned int TriSize = 3 * 3;
	const unsigned int MaxTrisPerJob = TRIS_PER_JOB * 6;
	const unsigned int MaxJobSize = MaxTrisPerJob * TriSize;
	mTrilistData = new float[MaxJobSize * mMaxJobs * mNumBins];

	// Setup trilist objects used for binning
	for (unsigned int i = 0; i < mMaxJobs; ++i)
	{
		for (unsigned int j = 0; j < mNumBins; ++j)
		{
			int idx = i*mNumBins + j;
			TriList &tList = mJobs[i].mRenderJobs[j];
			tList.mNumTriangles = MaxTrisPerJob;
			tList.mTriIdx = 0;
			tList.mPtr = mTrilistData + idx*MaxJobSize;
		}
	}

	// Clear render queue
	Reset();
}

CullingThreadpool::RenderJobQueue::~RenderJobQueue()
{
	SAFE_DELETE_ARRAY(mRenderPtrs);
	SAFE_DELETE_ARRAY(mBinMutexes);
	for (unsigned int i = 0; i < mMaxJobs; ++i)
		SAFE_DELETE_ARRAY(mJobs[i].mRenderJobs);
	SAFE_DELETE_ARRAY(mJobs);
	SAFE_DELETE_ARRAY(mTrilistData);
}

inline unsigned int CullingThreadpool::RenderJobQueue::GetMinRenderPtr() const
{
	unsigned int minRenderPtr = mRenderPtrs[0];
	for (unsigned int i = 1; i < mNumBins; ++i)
	{
		unsigned int renderPtr = mRenderPtrs[i];
		minRenderPtr = renderPtr < minRenderPtr ? renderPtr : minRenderPtr;
	}
	return minRenderPtr;
}

inline void CullingThreadpool::RenderJobQueue::AdvanceRenderJob(int binIdx)
{
	mRenderPtrs[binIdx]++;
	mBinMutexes[binIdx] = 0;
}

inline unsigned int CullingThreadpool::RenderJobQueue::GetBestGlobalQueue() const
{
	// Find least advanced queue
	unsigned int bestBin = ~0, bestPtr = mWritePtr;
	for (unsigned int i = 0; i < mNumBins; ++i)
	{
		if (mRenderPtrs[i] < bestPtr && mBinMutexes[i] == 0)
		{
			bestBin = i;
			bestPtr = mRenderPtrs[i];
		}
	}
	return bestBin;
}

inline bool CullingThreadpool::RenderJobQueue::IsPipelineEmpty() const
{
	return GetMinRenderPtr() == mWritePtr;
}

inline bool CullingThreadpool::RenderJobQueue::CanWrite() const
{
	return mWritePtr - GetMinRenderPtr() < mMaxJobs;
}

inline bool CullingThreadpool::RenderJobQueue::CanBin() const
{
	return mBinningPtr < mWritePtr && mBinningPtr - GetMinRenderPtr() < mMaxJobs;
}

inline CullingThreadpool::RenderJobQueue::Job *CullingThreadpool::RenderJobQueue::GetWriteJob()
{
	return &mJobs[mWritePtr % mMaxJobs];
}

inline void CullingThreadpool::RenderJobQueue::AdvanceWriteJob()
{
	mWritePtr++;
}

inline CullingThreadpool::RenderJobQueue::Job *CullingThreadpool::RenderJobQueue::GetBinningJob()
{
	unsigned int binningPtr = mBinningPtr;
	if (binningPtr < mWritePtr && binningPtr - GetMinRenderPtr() < mMaxJobs)
	{
		if (mBinningPtr.compare_exchange_strong(binningPtr, binningPtr + 1))
		{
			mJobs[binningPtr % mMaxJobs].mBinningJobStartedIdx = binningPtr;
			return &mJobs[binningPtr % mMaxJobs];
		}
	}
	return nullptr;
}

inline void CullingThreadpool::RenderJobQueue::FinishedBinningJob(Job *job)
{
	job->mBinningJobCompletedIdx = job->mBinningJobStartedIdx;
}

inline CullingThreadpool::RenderJobQueue::Job *CullingThreadpool::RenderJobQueue::GetRenderJob(int binIdx)
{
	// Attempt to lock bin mutex
	unsigned int expected = 0;
	if (!mBinMutexes[binIdx].compare_exchange_strong(expected, 1))
		return nullptr;

	// Check any items in the queue, and bail if empty
	if (mRenderPtrs[binIdx] != mJobs[mRenderPtrs[binIdx] % mMaxJobs].mBinningJobCompletedIdx)
	{
		mBinMutexes[binIdx] = 0;
		return nullptr;
	}

	return &mJobs[mRenderPtrs[binIdx] % mMaxJobs];
}

void CullingThreadpool::RenderJobQueue::Reset()
{
	mWritePtr = 0;
	mBinningPtr = 0;

	for (unsigned int i = 0; i < mNumBins; ++i)
		mRenderPtrs[i] = 0;

	for (unsigned int i = 0; i < mMaxJobs; ++i)
	{
		mJobs[i].mBinningJobCompletedIdx = -1;
		mJobs[i].mBinningJobStartedIdx = -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Culling threadpool private helper functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CullingThreadpool::SetupScissors()
{
	unsigned int width, height;
	mMOC->GetResolution(width, height);

	unsigned int binWidth;
	unsigned int binHeight;
	mMOC->ComputeBinWidthHeight(mBinsW, mBinsH, binWidth, binHeight);

	for (unsigned int ty = 0; ty < mBinsH; ++ty)
	{
		for (unsigned int tx = 0; tx < mBinsW; ++tx)
		{
			unsigned int threadIdx = tx + ty*mBinsW;

			// Adjust rects on final row / col to match resolution
			mRects[threadIdx].mMinX = tx*binWidth;
			mRects[threadIdx].mMaxX = tx + 1 == mBinsW ? width : (tx + 1) * binWidth;
			mRects[threadIdx].mMinY = ty * binHeight;
			mRects[threadIdx].mMaxY = ty + 1 == mBinsH ? height : (ty + 1) * binHeight;
		}
	}
}

void CullingThreadpool::ThreadRun(CullingThreadpool *threadPool, unsigned int threadId)
{ 
	threadPool->ThreadMain(threadId); 
}

void CullingThreadpool::ThreadMain(unsigned int threadIdx)
{
	while (true)
	{
		bool threadIsIdle = true;
		unsigned int threadBinIdx = threadIdx;

		// Wait for threads to be woken up (low CPU load sleep)
		std::unique_lock<std::mutex> lock(mSuspendedMutex);
		mNumSuspendedThreads++;
		mSuspendedCV.wait(lock, [&] {return !mSuspendThreads; });
		mNumSuspendedThreads--;
		lock.unlock();

		// Loop until suspended again
		while (!mSuspendThreads || !threadIsIdle)
		{
			if (mKillThreads)
				return;

			threadIsIdle = false;

			// Prio 1: Process any render jobs local to this thread
			unsigned int binIdx = threadBinIdx;
			threadBinIdx = threadBinIdx + mNumThreads < mNumBins ? threadBinIdx + mNumThreads : threadIdx;
			RenderJobQueue::Job *job = mRenderQueue->GetRenderJob(binIdx);
			if (job != nullptr)
			{
				if (job->mRenderJobs[binIdx].mTriIdx > 0)
					mMOC->RenderTrilist(job->mRenderJobs[binIdx], &mRects[binIdx]);

				mRenderQueue->AdvanceRenderJob(binIdx);
				continue;
			}

			// Prio 2: Process any outstanding setup/binning jobs
			if (mRenderQueue->CanBin())
			{
				// If no more rasterization jobs, get next binning job
				RenderJobQueue::Job *job = mRenderQueue->GetBinningJob();
				if (job != nullptr)
				{
					RenderJobQueue::BinningJob &sjob = job->mBinningJob;
					for (unsigned int i = 0; i < mNumBins; ++i)
						job->mRenderJobs[i].mTriIdx = 0;
					mMOC->BinTriangles(sjob.mVerts, sjob.mTris, sjob.nTris, job->mRenderJobs, mBinsW, mBinsH, sjob.mMatrix, sjob.mBfWinding, sjob.mClipPlanes, *sjob.mVtxLayout);
					mRenderQueue->FinishedBinningJob(job);
				}
				continue;
			}

			// Prio 3: No work is available, work steal from another thread's queue
			if (mNumBins > mNumThreads)
			{
				binIdx = mRenderQueue->GetBestGlobalQueue();
				if (binIdx < mRenderQueue->mNumBins)
				{
					RenderJobQueue::Job *job = mRenderQueue->GetRenderJob(binIdx);
					if (job != nullptr)
					{
						if (job->mRenderJobs[binIdx].mTriIdx > 0)
							mMOC->RenderTrilist(job->mRenderJobs[binIdx], &mRects[binIdx]);

						mRenderQueue->AdvanceRenderJob(binIdx);
					}
					continue;
				}
			}

			// No work available: Yield this thread
			std::this_thread::yield();
			threadIsIdle = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Culling threadpool public API, similar to the MaskedOcclusionCulling class
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CullingThreadpool::CullingThreadpool(unsigned int numThreads, unsigned int binsW, unsigned int binsH, unsigned int maxJobs) :
	mNumThreads(numThreads),
	mMaxJobs(maxJobs),
	mBinsW(binsW),
	mBinsH(binsH),
	mKillThreads(false),
	mSuspendThreads(true),
	mNumSuspendedThreads(0),
	mModelToClipMatrices(maxJobs),
	mVertexLayouts(maxJobs),
	mMOC(nullptr)
{
	mNumBins = mBinsW*mBinsH;
	assert(mNumBins >= mNumThreads);	// Having less bins than threads is a bad idea!

	mRects = new ScissorRect[mNumBins];
	mRenderQueue = new RenderJobQueue(mNumBins, mMaxJobs);

	// Add default vertex layout and matrix
	mVertexLayouts.AddData(VertexLayout(16, 4, 12));
	mCurrentMatrix = nullptr;

	mThreads = new std::thread[mNumThreads];
	for (unsigned int i = 0; i < mNumThreads; ++i)
		mThreads[i] = std::thread(ThreadRun, this, i);

}

CullingThreadpool::~CullingThreadpool()
{
	// Wait for threads to terminate
	if (mThreads != nullptr || !mKillThreads)
	{
		WakeThreads();
		mKillThreads = true;
		for (unsigned int i = 0; i < mNumThreads; ++i)
			mThreads[i].join();

	}

	// Free memory
	SAFE_DELETE(mRenderQueue);
	SAFE_DELETE_ARRAY(mRects);
	SAFE_DELETE_ARRAY(mThreads);
}

void CullingThreadpool::WakeThreads()
{
	// Wait for all threads to be in suspended mode
	while (mNumSuspendedThreads < mNumThreads)
		std::this_thread::yield();

	// Send wake up event
	std::unique_lock<std::mutex> lock(mSuspendedMutex);
	mSuspendThreads = false;
	lock.unlock();
	mSuspendedCV.notify_all();
}

void CullingThreadpool::SuspendThreads()
{
	// Signal threads to go into suspended mode (after finishing all outstanding work)
	mSuspendThreads = true;
}

void CullingThreadpool::Flush()
{
	// Wait for pipeline to be empty (i.e. all work is finished)
	while (!mRenderQueue->IsPipelineEmpty())
		std::this_thread::yield();

	// Reset queue counters
	mRenderQueue->Reset();
}

void CullingThreadpool::SetBuffer(MaskedOcclusionCulling *moc)
{
	Flush();
	mMOC = moc;
	SetupScissors();
}

void CullingThreadpool::SetResolution(unsigned int width, unsigned int height)
{
	Flush();
	mMOC->SetResolution(width, height);
	SetupScissors();
}

void CullingThreadpool::SetNearClipPlane(float nearDist)
{
	Flush();
	mMOC->SetNearClipPlane(nearDist);
}

void CullingThreadpool::SetMatrix(const float *modelToClipMatrix)
{
	// Treat nullptr matrix as a special case, otherwise copy the contents of the pointer and add to state
	if (modelToClipMatrix == nullptr)
		mCurrentMatrix = nullptr;
	else
	{
		mModelToClipMatrices.AddData(Matrix4x4(modelToClipMatrix));
		mCurrentMatrix = mModelToClipMatrices.GetData()->mValues;
	}
}

void CullingThreadpool::SetVertexLayout(const VertexLayout &vtxLayout)
{
	mVertexLayouts.AddData(vtxLayout);
}

void CullingThreadpool::ClearBuffer()
{
	Flush();
	mMOC->ClearBuffer();
}

void CullingThreadpool::RenderTriangles(const float *inVtx, const unsigned int *inTris, int nTris, BackfaceWinding bfWinding, ClipPlanes clipPlaneMask)
{
#if MOC_RECORDER_ENABLE != 0
    mMOC->RecordRenderTriangles( inVtx, inTris, nTris, mCurrentMatrix, clipPlaneMask, bfWinding, *mVertexLayouts.GetData( ) );
#endif

    for (int i = 0; i < nTris; i += TRIS_PER_JOB)
	{
		// Yield if work queue is full 
		while (!mRenderQueue->CanWrite())
			std::this_thread::yield();

		// Create new renderjob
		RenderJobQueue::Job *job = mRenderQueue->GetWriteJob();
		job->mBinningJob.mVerts = inVtx;
		job->mBinningJob.mTris = inTris + i * 3;
		job->mBinningJob.nTris = nTris - i < TRIS_PER_JOB ? nTris - i : TRIS_PER_JOB;
		job->mBinningJob.mMatrix = mCurrentMatrix;
		job->mBinningJob.mClipPlanes = clipPlaneMask;
		job->mBinningJob.mBfWinding = bfWinding;
		job->mBinningJob.mVtxLayout = mVertexLayouts.GetData();
		mRenderQueue->AdvanceWriteJob();
	}
}

CullingThreadpool::CullingResult CullingThreadpool::TestRect(float xmin, float ymin, float xmax, float ymax, float wmin)
{
	return mMOC->TestRect(xmin, ymin, xmax, ymax, wmin);
}

CullingThreadpool::CullingResult CullingThreadpool::TestTriangles(const float *inVtx, const unsigned int *inTris, int nTris, BackfaceWinding bfWinding, ClipPlanes clipPlaneMask)
{
	return mMOC->TestTriangles(inVtx, inTris, nTris, mCurrentMatrix, bfWinding, clipPlaneMask, *mVertexLayouts.GetData());
}

void CullingThreadpool::ComputePixelDepthBuffer(float *depthData, bool flipY)
{
	Flush();
	mMOC->ComputePixelDepthBuffer(depthData, flipY);
}
