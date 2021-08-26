/**
 * MojoAL; a simple drop-in OpenAL implementation.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#include <stdio.h>
#include <stdlib.h>  /* needed for alloca */
#include <math.h>
#include <float.h>

#ifdef _WIN32
  #define AL_API __declspec(dllexport)
  #define ALC_API __declspec(dllexport)
#endif

#ifndef M_PI
  #define M_PI (3.14159265358979323846264338327950288)
#endif

#include "AL/al.h"
#include "AL/alc.h"
#include "SDL.h"
#include "samplerate.h"

#ifdef __SSE__  /* if you are on x86 or x86-64, we assume you have SSE1 by now. */
#define NEED_SCALAR_FALLBACK 0
#elif (defined(__ARM_ARCH) && (__ARM_ARCH >= 8))  /* ARMv8 always has NEON. */
#define NEED_SCALAR_FALLBACK 0
#elif (defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7))   /* All ARMv7 chips from Apple have NEON. */
#define NEED_SCALAR_FALLBACK 0
#elif (defined(__WINDOWS__) || defined(__WINRT__)) && defined(_M_ARM)  /* all WinRT-level Microsoft devices have NEON */
#define NEED_SCALAR_FALLBACK 0
#else
#define NEED_SCALAR_FALLBACK 1
#endif

/* Some platforms fail to define __ARM_NEON__, others need it or arm_neon.h will fail. */
#if (defined(__ARM_ARCH) || defined(_M_ARM))
#  if !NEED_SCALAR_FALLBACK && !defined(__ARM_NEON__)
#    define __ARM_NEON__ 1
#  endif
#endif

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

#define OPENAL_VERSION_MAJOR 1
#define OPENAL_VERSION_MINOR 1
#define OPENAL_VERSION_STRING3(major, minor) #major "." #minor
#define OPENAL_VERSION_STRING2(major, minor) OPENAL_VERSION_STRING3(major, minor)

/* !!! FIXME: make some decisions about VENDOR and RENDERER strings here */
#define OPENAL_VERSION_STRING OPENAL_VERSION_STRING2(OPENAL_VERSION_MAJOR, OPENAL_VERSION_MINOR)
#define OPENAL_VENDOR_STRING "Ryan C. Gordon"
#define OPENAL_RENDERER_STRING "mojoAL"

#define DEFAULT_PLAYBACK_DEVICE "Default OpenAL playback device"
#define DEFAULT_CAPTURE_DEVICE "Default OpenAL capture device"

/* Number of buffers to allocate at once when we need a new block during alGenBuffers(). */
#ifndef OPENAL_BUFFER_BLOCK_SIZE
#define OPENAL_BUFFER_BLOCK_SIZE 256
#endif

/* Number of sources to allocate at once when we need a new block during alGenSources(). */
#ifndef OPENAL_SOURCE_BLOCK_SIZE
#define OPENAL_SOURCE_BLOCK_SIZE 64
#endif

/* AL_EXT_FLOAT32 support... */
#ifndef AL_FORMAT_MONO_FLOAT32
#define AL_FORMAT_MONO_FLOAT32 0x10010
#endif

#ifndef AL_FORMAT_STEREO_FLOAT32
#define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif

/* ALC_EXT_DISCONNECTED support... */
#ifndef ALC_CONNECTED
#define ALC_CONNECTED 0x313
#endif


/*
The locking strategy for this OpenAL implementation:

- The initial work on this implementation attempted to be completely
  lock free, and it lead to fragile, overly-clever, and complicated code.
  Attempt #2 is making more reasonable tradeoffs.

- All API entry points are protected by a global mutex, which means that
  calls into the API are serialized, but we expect this to not be a
  serious problem; most AL calls are likely to come from a single thread
  and uncontended mutexes generally aren't very expensive. This mutex
  is not shared with the mixer thread, so there is never a point where
  an innocent "fast" call into the AL will block because of the bad luck
  of a high mixing load and the wrong moment.

- In rare cases we'll lock the mixer thread for a brief time; when a playing
  source is accessible to the mixer, it is flagged as such. The mixer has a
  mutex that it holds when mixing a source, and if we need to touch a source
  that is flagged as accessible, we'll grab that lock to make sure there isn't
  a conflict. Not all source changes need to do this. The likelihood of
  hitting this case is extremely small, and the lock hold time is pretty
  short. Things that might do this, only on currently-playing sources:
  alDeleteSources, alSourceStop, alSourceRewind. alSourcePlay and
  alSourcePause never need to lock.

- Devices are expected to live for the entire life of your OpenAL
  experience, so closing one while another thread is using it is your own
  fault. Don't do that. Devices are allocated pointers, and the AL doesn't
  know if you've deleted it, making the pointer invalid. Device open and
  close are not meant to be "fast" calls.

- Creating or destroying a context will lock the mixer thread completely
  (so it isn't running _at all_ during the lock), so we can add/remove the
  context on the device's list without racing. So don't do this in
  time-critical code.

- Generating an object (source, buffer, etc) might need to allocate
  memory, which can always take longer than you would expect. We allocate in
  blocks, so not every call will allocate more memory. Generating an object
  does not lock the mixer thread.

- Deleting a buffer does not lock the mixer thread (in-use buffers can
  not be deleted per API spec). Deleting a source will lock the mixer briefly
  if the source is still visible to the mixer. We don't believe this will be
  a serious issue in normal use cases. Deleted objects' memory is marked for
  reuse, but no memory is free'd by deleting sources or buffers until the
  context or device, respectively, are destroyed. A deleted source that's
  still visible to the mixer will not be available for reallocation until
  the mixer runs another iteration, where it will mark it as no longer
  visible. If you call alGenSources() during this time, a different source
  will be allocated.

- alBufferData needs to allocate memory to copy new audio data. Often,
  you can avoid doing these things in time-critical code. You can't set
  a buffer's data when it's attached to a source (either with AL_BUFFER
  or buffer queueing), so there's never a chance of contention with the
  mixer thread here.

- Buffers and sources are allocated in blocks of OPENAL_BUFFER_BLOCK_SIZE
  (or OPENAL_SOURCE_BLOCK_SIZE). These blocks are never deallocated as long
  as the device (for buffers) or context (for sources) lives, so they don't
  need a lock to access as the pointers are immutable once they're wired in.
  We don't keep a ALuint name index array, but rather an array of block
  pointers, which lets us find the right offset in the correct block without
  iteration. The mixer thread never references the blocks directly, as they
  get buffer and source pointers to objects within those blocks. Sources keep
  a pointer to their specifically-bound buffer, and the mixer keeps a list of
  pointers to playing sources. Since the API is serialized and the mixer
  doesn't touch them, we don't need to tapdance to add new blocks.

- Buffer data is owned by the AL, and it's illegal to delete a buffer or
  alBufferData() its contents while attached to a source with either
  AL_BUFFER or alSourceQueueBuffers(). We keep an atomic refcount for each
  buffer, and you can't change its state or delete it when its refcount is
  > 0, so there isn't a race with the mixer. Refcounts only change when
  changing a source's AL_BUFFER or altering its buffer queue, both of which
  are protected by the api lock. The mixer thread doesn't touch the
  refcount, as a buffer moving from AL_PENDING to AL_PROCESSED is still
  attached to a source.

- alSource(Stop|Pause|Rewind)v with > 1 source used will always lock the
  mixer thread to guarantee that all sources change in sync (!!! FIXME?).
  The non-v version of these functions do not lock the mixer thread.
  alSourcePlayv never locks the mixer thread (it atomically appends to a
  linked list of sources to be played, which the mixer will pick up all
  at once).

- alSourceQueueBuffers will build a linked list of buffers, then atomically
  move this list into position for the mixer to obtain it. The mixer will
  process this list without the need to be atomic (as it owns it once it
  atomically claims it from from the just_queued field where
  alSourceQueueBuffers staged it). As buffers are processed, the mixer moves
  them atomically to a linked list that other threads can pick up for
  alSourceUnqueueBuffers.

- Capture just locks the SDL audio device for everything, since it's a very
  lightweight load and a much simplified API; good enough. The capture device
  thread is an almost-constant minimal load (1 or 2 memcpy's, depending on the
  ring buffer position), and the worst load on the API side (alcCaptureSamples)
  is the same deal, so this never takes long, and is good enough.

- Probably other things. These notes might get updates later.
*/

#if 1
#define FIXME(x)
#else
#define FIXME(x) { \
    static int seen = 0; \
    if (!seen) { \
        seen = 1; \
        fprintf(stderr, "FIXME: %s (%s@%s:%d)\n", x, __FUNCTION__, __FILE__, __LINE__); \
    } \
}
#endif

/* restrict is from C99, but __restrict works with both Visual Studio and GCC. */
#if !defined(restrict) && ((!defined(__STDC_VERSION__) || (__STDC_VERSION__ < 199901)))
#define restrict __restrict
#endif

#ifdef _MSC_VER
#define SIMDALIGNEDSTRUCT __declspec(align(16)) struct
#elif (defined(__GNUC__) || defined(__clang__))
#define SIMDALIGNEDSTRUCT struct __attribute__((aligned(16)))
#else
#define SIMDALIGNEDSTRUCT struct
#endif

#ifdef __SSE__  /* we assume you always have this on x86/x86-64 chips. SSE1 is 20 years old! */
#define has_sse 1
#endif

#ifdef __ARM_NEON__
#if NEED_SCALAR_FALLBACK
static int has_neon = 0;
#else
#define has_neon 1
#endif
#endif

static SDL_mutex *api_lock = NULL;

static int init_api_lock(void)
{
    if (!api_lock) {
        api_lock = SDL_CreateMutex();
        if (!api_lock) {
            return 0;
        }
    }
    return 1;
}

static void grab_api_lock(void)
{
    if (!api_lock) {
        if (!init_api_lock()) {
            return;
        }
    }
    const int rc = SDL_LockMutex(api_lock);
    SDL_assert(rc == 0);
}

static void ungrab_api_lock(void)
{
    if (!api_lock) {
        init_api_lock();
        return;
    }

    const int rc = SDL_UnlockMutex(api_lock);
    SDL_assert(rc == 0);
}

#define ENTRYPOINT(rettype,fn,params,args) \
    rettype fn params { rettype retval; grab_api_lock(); retval = _##fn args ; ungrab_api_lock(); return retval; }

#define ENTRYPOINTVOID(fn,params,args) \
    void fn params { grab_api_lock(); _##fn args ; ungrab_api_lock(); }


/* lifted this ring buffer code from my al_osx project; I wrote it all, so it's stealable. */
typedef struct
{
    ALCubyte *buffer;
    ALCsizei size;
    ALCsizei write;
    ALCsizei read;
    ALCsizei used;
} RingBuffer;

static void ring_buffer_put(RingBuffer *ring, const void *_data, const ALCsizei size)
{
    const ALCubyte *data = (const ALCubyte *) _data;
    ALCsizei cpy;
    ALCsizei avail;

    if (!size)   /* just in case... */
        return;

    /* Putting more data than ring buffer holds in total? Replace it all. */
    if (size > ring->size) {
        ring->write = 0;
        ring->read = 0;
        ring->used = ring->size;
        SDL_memcpy(ring->buffer, data + (size - ring->size), ring->size);
        return;
    }

    /* Buffer overflow? Push read pointer to oldest sample not overwritten... */
    avail = ring->size - ring->used;
    if (size > avail) {
        ring->read += size - avail;
        if (ring->read > ring->size)
            ring->read -= ring->size;
    }

    /* Clip to end of buffer and copy first block... */
    cpy = ring->size - ring->write;
    if (size < cpy)
        cpy = size;
    if (cpy) SDL_memcpy(ring->buffer + ring->write, data, cpy);

    /* Wrap around to front of ring buffer and copy remaining data... */
    avail = size - cpy;
    if (avail) SDL_memcpy(ring->buffer, data + cpy, avail);

    /* Update write pointer... */
    ring->write += size;
    if (ring->write > ring->size)
        ring->write -= ring->size;

    ring->used += size;
    if (ring->used > ring->size)
        ring->used = ring->size;
}


static ALCsizei ring_buffer_get(RingBuffer *ring, void *_data, ALCsizei size)
{
    ALCubyte *data = (ALCubyte *) _data;
    ALCsizei cpy;
    ALCsizei avail = ring->used;

    /* Clamp amount to read to available data... */
    if (size > avail)
        size = avail;

    /* Clip to end of buffer and copy first block... */
    cpy = ring->size - ring->read;
    if (cpy > size) cpy = size;
    if (cpy) SDL_memcpy(data, ring->buffer + ring->read, cpy);

    /* Wrap around to front of ring buffer and copy remaining data... */
    avail = size - cpy;
    if (avail) SDL_memcpy(data + cpy, ring->buffer, avail);

    /* Update read pointer... */
    ring->read += size;
    if (ring->read > ring->size)
        ring->read -= ring->size;

    ring->used -= size;

    return size;  /* may have been clamped if there wasn't enough data... */
}

static void *calloc_simd_aligned(const size_t len)
{
    Uint8 *retval = NULL;
    Uint8 *ptr = (Uint8 *) SDL_calloc(1, len + 16 + sizeof (void *));
    if (ptr) {
        void **storeptr;
        retval = ptr + sizeof (void *);
        retval += 16 - (((size_t) retval) % 16);
        storeptr = (void **) retval;
        storeptr--;
        *storeptr = ptr;
    }
    return retval;
}

static void free_simd_aligned(void *ptr)
{
    if (ptr) {
        void **realptr = (void **) ptr;
        realptr--;
        SDL_free(*realptr);
    }
}


typedef struct ALbuffer
{
    ALboolean allocated;
    ALuint name;
    ALint channels;
    ALint bits;  /* always float32 internally, but this is what alBufferData saw */
    ALsizei frequency;
    ALsizei len;   /* length of data in bytes. */
    const float *data;  /* we only work in Float32 format. */
    SDL_atomic_t refcount;  /* if zero, can be deleted or alBufferData'd */
} ALbuffer;

/* !!! FIXME: buffers and sources use almost identical code for blocks */
typedef struct BufferBlock
{
    ALbuffer buffers[OPENAL_BUFFER_BLOCK_SIZE];  /* allocate these in blocks so we can step through faster. */
    ALuint used;
    ALuint tmp;  /* only touch under api_lock, assume it'll be gone later. */
} BufferBlock;

typedef struct BufferQueueItem
{
    ALbuffer *buffer;
    void *next;  /* void* because we'll atomicgetptr it. */
} BufferQueueItem;

typedef struct BufferQueue
{
    void *just_queued;  /* void* because we'll atomicgetptr it. */
    BufferQueueItem *head;
    BufferQueueItem *tail;
    SDL_atomic_t num_items;  /* counts just_queued+head/tail */
} BufferQueue;

typedef struct ALsource ALsource;

SIMDALIGNEDSTRUCT ALsource
{
    /* keep these first to help guarantee that its elements are aligned for SIMD */
    ALfloat position[4];
    ALfloat velocity[4];
    ALfloat direction[4];
    ALfloat panning[2];  /* we only do stereo for now */
    SDL_atomic_t mixer_accessible;
    SDL_atomic_t state;  /* initial, playing, paused, stopped */
    ALuint name;
    ALboolean allocated;
    ALenum type;  /* undetermined, static, streaming */
    ALboolean recalc;
    ALboolean source_relative;
    ALboolean looping;
    ALfloat gain;
    ALfloat min_gain;
    ALfloat max_gain;
    ALfloat reference_distance;
    ALfloat max_distance;
    ALfloat rolloff_factor;
    ALfloat pitch;
    ALfloat cone_inner_angle;
    ALfloat cone_outer_angle;
    ALfloat cone_outer_gain;
    ALbuffer *buffer;
    SRC_STATE *src_state;  /* for resampling. */
    BufferQueue buffer_queue;
    BufferQueue buffer_queue_processed;
    ALsizei offset;  /* offset in bytes for converted stream! */
    ALboolean offset_latched;  /* AL_SEC_OFFSET, etc, say set values apply to next alSourcePlay if not currently playing! */
    ALint queue_channels;
    ALsizei queue_frequency;
    ALsource *playlist_next;  /* linked list that contains currently-playing sources! Only touched by mixer thread! */
};

/* !!! FIXME: buffers and sources use almost identical code for blocks */
typedef struct SourceBlock
{
    ALsource sources[OPENAL_SOURCE_BLOCK_SIZE];  /* allocate these in blocks so we can step through faster. */
    ALuint used;
    ALuint tmp;  /* only touch under api_lock, assume it'll be gone later. */
} SourceBlock;


typedef struct SourcePlayTodo
{
    ALsource *source;
    struct SourcePlayTodo *next;
} SourcePlayTodo;

struct ALCdevice_struct
{
    char *name;
    ALCenum error;
    SDL_atomic_t connected;
    ALCboolean iscapture;
    SDL_AudioDeviceID sdldevice;

    ALint channels;
    ALint frequency;
    ALCsizei framesize;

    union {
        struct {
            ALCcontext *contexts;
            BufferBlock **buffer_blocks;  /* buffers are shared between contexts on the same device. */
            ALCsizei num_buffer_blocks;
            BufferQueueItem *buffer_queue_pool;  /* mixer thread doesn't touch this. */
            void *source_todo_pool;  /* void* because we'll atomicgetptr it. */
        } playback;
        struct {
            RingBuffer ring;  /* only used if iscapture */
        } capture;
    };
};

struct ALCcontext_struct
{
    /* keep these first to help guarantee that its elements are aligned for SIMD */
    SourceBlock **source_blocks;
    ALsizei num_source_blocks;

    SIMDALIGNEDSTRUCT {
        ALfloat position[4];
        ALfloat velocity[4];
        ALfloat orientation[8];
        ALfloat gain;
    } listener;

    ALCdevice *device;
    SDL_atomic_t processing;
    ALenum error;
    ALCint *attributes;
    ALCsizei attributes_count;

    ALCboolean recalc;
    ALenum distance_model;
    ALfloat doppler_factor;
    ALfloat doppler_velocity;
    ALfloat speed_of_sound;

    SDL_mutex *source_lock;

    void *playlist_todo;  /* void* so we can AtomicCASPtr it. Transmits new play commands from api thread to mixer thread */
    ALsource *playlist;  /* linked list of currently-playing sources. Mixer thread only! */
    ALsource *playlist_tail;  /* end of playlist so we know if last item is being readded. Mixer thread only! */

    ALCcontext *prev;  /* contexts are in a double-linked list */
    ALCcontext *next;
};

/* forward declarations */
static float source_get_offset(ALsource *src, ALenum param);
static void source_set_offset(ALsource *src, ALenum param, ALfloat value);

/* the just_queued list is backwards. Add it to the queue in the correct order. */
static void queue_new_buffer_items_recursive(BufferQueue *queue, BufferQueueItem *items)
{
    if (items == NULL) {
        return;
    }

    queue_new_buffer_items_recursive(queue, items->next);
    items->next = NULL;
    if (queue->tail) {
        queue->tail->next = items;
    } else {
        queue->head = items;
    }
    queue->tail = items;
}

static void obtain_newly_queued_buffers(BufferQueue *queue)
{
    BufferQueueItem *items;
    do {
        items = (BufferQueueItem *) SDL_AtomicGetPtr(&queue->just_queued);
    } while (!SDL_AtomicCASPtr(&queue->just_queued, items, NULL));

    /* Now that we own this pointer, we can just do whatever we want with it.
       Nothing touches the head/tail fields other than the mixer thread, so we
       move it there. Not even atomically!  :) */
    SDL_assert((queue->tail != NULL) == (queue->head != NULL));

    queue_new_buffer_items_recursive(queue, items);
}

/* You probably need to hold a lock before you call this (currently). */
static void source_mark_all_buffers_processed(ALsource *src)
{
    obtain_newly_queued_buffers(&src->buffer_queue);
    while (src->buffer_queue.head) {
        void *ptr;
        BufferQueueItem *item = src->buffer_queue.head;
        src->buffer_queue.head = item->next;
        SDL_AtomicAdd(&src->buffer_queue.num_items, -1);

        /* Move it to the processed queue for alSourceUnqueueBuffers() to pick up. */
        do {
            ptr = SDL_AtomicGetPtr(&src->buffer_queue_processed.just_queued);
            SDL_AtomicSetPtr(&item->next, ptr);
        } while (!SDL_AtomicCASPtr(&src->buffer_queue_processed.just_queued, ptr, item));

        SDL_AtomicAdd(&src->buffer_queue_processed.num_items, 1);
    }
    src->buffer_queue.tail = NULL;
}

static void source_release_buffer_queue(ALCcontext *ctx, ALsource *src)
{
    /* move any buffer queue items to the device's available pool for reuse. */
    obtain_newly_queued_buffers(&src->buffer_queue);
    if (src->buffer_queue.tail != NULL) {
        BufferQueueItem *i;
        for (i = src->buffer_queue.head; i; i = i->next) {
            (void) SDL_AtomicDecRef(&i->buffer->refcount);
        }
        src->buffer_queue.tail->next = ctx->device->playback.buffer_queue_pool;
        ctx->device->playback.buffer_queue_pool = src->buffer_queue.head;
    }
    src->buffer_queue.head = src->buffer_queue.tail = NULL;
    SDL_AtomicSet(&src->buffer_queue.num_items, 0);

    obtain_newly_queued_buffers(&src->buffer_queue_processed);
    if (src->buffer_queue_processed.tail != NULL) {
        BufferQueueItem *i;
        for (i = src->buffer_queue_processed.head; i; i = i->next) {
            (void) SDL_AtomicDecRef(&i->buffer->refcount);
        }
        src->buffer_queue_processed.tail->next = ctx->device->playback.buffer_queue_pool;
        ctx->device->playback.buffer_queue_pool = src->buffer_queue_processed.head;
    }
    src->buffer_queue_processed.head = src->buffer_queue_processed.tail = NULL;
    SDL_AtomicSet(&src->buffer_queue_processed.num_items, 0);
}


/* ALC implementation... */

static void *current_context = NULL;
static ALCenum null_device_error = ALC_NO_ERROR;

/* we don't have any device-specific extensions. */
#define ALC_EXTENSION_ITEMS \
    ALC_EXTENSION_ITEM(ALC_ENUMERATION_EXT) \
    ALC_EXTENSION_ITEM(ALC_EXT_CAPTURE) \
    ALC_EXTENSION_ITEM(ALC_EXT_DISCONNECT)

#define AL_EXTENSION_ITEMS \
    AL_EXTENSION_ITEM(AL_EXT_FLOAT32)


static void set_alc_error(ALCdevice *device, const ALCenum error)
{
    ALCenum *perr = device ? &device->error : &null_device_error;
    /* can't set a new error when the previous hasn't been cleared yet. */
    if (*perr == ALC_NO_ERROR) {
        *perr = error;
    }
}

/* all data written before the release barrier must be available before the recalc flag changes. */ \
#define context_needs_recalc(ctx) SDL_MemoryBarrierRelease(); ctx->recalc = AL_TRUE;
#define source_needs_recalc(src) SDL_MemoryBarrierRelease(); src->recalc = AL_TRUE;

static ALCdevice *prep_alc_device(const char *devicename, const ALCboolean iscapture)
{
    ALCdevice *dev = NULL;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
        return NULL;
    }

    #ifdef __SSE__
    if (!SDL_HasSSE()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;  /* whoa! Better order a new Pentium III from Gateway 2000! */
    }
    #endif

    #if defined(__ARM_NEON__) && !NEED_SCALAR_FALLBACK
    if (!SDL_HasNEON()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;  /* :( */
    }
    #elif defined(__ARM_NEON__) && NEED_SCALAR_FALLBACK
    has_neon = SDL_HasNEON();
    #endif

    if (!init_api_lock()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    dev = (ALCdevice *) SDL_calloc(1, sizeof (ALCdevice));
    if (!dev) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    dev->name = SDL_strdup(devicename);
    if (!dev->name) {
        SDL_free(dev);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    SDL_AtomicSet(&dev->connected, ALC_TRUE);
    dev->iscapture = iscapture;

    return dev;
}

/* no api lock; this creates it and otherwise doesn't have any state that can race */
ALCdevice *alcOpenDevice(const ALCchar *devicename)
{
    if (!devicename) {
        devicename = DEFAULT_PLAYBACK_DEVICE;  /* so ALC_DEVICE_SPECIFIER is meaningful */
    }

    return prep_alc_device(devicename, ALC_FALSE);

    /* we don't open an SDL audio device until the first context is
       created, so we can attempt to match audio formats. */
}

/* no api lock; this requires you to not destroy a device that's still in use */
ALCboolean alcCloseDevice(ALCdevice *device)
{
    BufferQueueItem *item;
    SourcePlayTodo *todo;
    ALCsizei i;

    if (!device || device->iscapture) {
        return ALC_FALSE;
    }

    /* spec: "Failure will occur if all the device's contexts and buffers have not been destroyed." */
    if (device->playback.contexts) {
        return ALC_FALSE;
    }

    for (i = 0; i <device->playback.num_buffer_blocks; i++) {
        if (device->playback.buffer_blocks[i]->used > 0) {
            return ALC_FALSE;  /* still buffers allocated. */
        }
    }

    if (device->sdldevice) {
        SDL_CloseAudioDevice(device->sdldevice);
    }

    for (i = 0; i < device->playback.num_buffer_blocks; i++) {
        SDL_free(device->playback.buffer_blocks[i]);
    }
    SDL_free(device->playback.buffer_blocks);

    item = device->playback.buffer_queue_pool;
    while (item) {
        BufferQueueItem *next = item->next;
        SDL_free(item);
        item = next;
    }

    todo = (SourcePlayTodo *) device->playback.source_todo_pool;
    while (todo) {
        SourcePlayTodo *next = todo->next;
        SDL_free(todo);
        todo = next;
    }

    SDL_free(device->name);
    SDL_free(device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return ALC_TRUE;
}


static ALCboolean alcfmt_to_sdlfmt(const ALCenum alfmt, SDL_AudioFormat *sdlfmt, Uint8 *channels, ALCsizei *framesize)
{
    switch (alfmt) {
        case AL_FORMAT_MONO8:
            *sdlfmt = AUDIO_U8;
            *channels = 1;
            *framesize = 1;
            break;
        case AL_FORMAT_MONO16:
            *sdlfmt = AUDIO_S16SYS;
            *channels = 1;
            *framesize = 2;
            break;
        case AL_FORMAT_STEREO8:
            *sdlfmt = AUDIO_U8;
            *channels = 2;
            *framesize = 2;
            break;
        case AL_FORMAT_STEREO16:
            *sdlfmt = AUDIO_S16SYS;
            *channels = 2;
            *framesize = 4;
            break;
        case AL_FORMAT_MONO_FLOAT32:
            *sdlfmt = AUDIO_F32SYS;
            *channels = 1;
            *framesize = 4;
            break;
        case AL_FORMAT_STEREO_FLOAT32:
            *sdlfmt = AUDIO_F32SYS;
            *channels = 2;
            *framesize = 8;
            break;
        default:
            return ALC_FALSE;
    }

    return ALC_TRUE;
}

static void mix_float32_c1_scalar(const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 4;
    const int leftover = mixframes % 4;
    ALsizei i;

    if ((left == 1.0f) && (right == 1.0f)) {
        for (i = 0; i < unrolled; i++, data += 4, stream += 8) {
            const float samp0 = data[0];
            const float samp1 = data[1];
            const float samp2 = data[2];
            const float samp3 = data[3];
            stream[0] += samp0;
            stream[1] += samp0;
            stream[2] += samp1;
            stream[3] += samp1;
            stream[4] += samp2;
            stream[5] += samp2;
            stream[6] += samp3;
            stream[7] += samp3;
        }
        for (i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp;
            stream[1] += samp;
        }
    } else {
        for (i = 0; i < unrolled; i++, data += 4, stream += 8) {
            const float samp0 = data[0];
            const float samp1 = data[1];
            const float samp2 = data[2];
            const float samp3 = data[3];
            stream[0] += samp0 * left;
            stream[1] += samp0 * right;
            stream[2] += samp1 * left;
            stream[3] += samp1 * right;
            stream[4] += samp2 * left;
            stream[5] += samp2 * right;
            stream[6] += samp3 * left;
            stream[7] += samp3 * right;
        }
        for (i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp * left;
            stream[1] += samp * right;
        }
    }
}

static void mix_float32_c2_scalar(const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 4;
    const int leftover = mixframes % 4;
    ALsizei i;

    if ((left == 1.0f) && (right == 1.0f)) {
        for (i = 0; i < unrolled; i++, stream += 8, data += 8) {
            stream[0] += data[0];
            stream[1] += data[1];
            stream[2] += data[2];
            stream[3] += data[3];
            stream[4] += data[4];
            stream[5] += data[5];
            stream[6] += data[6];
            stream[7] += data[7];
        }
        for (i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0];
            stream[1] += data[1];
        }
    } else {
        for (i = 0; i < unrolled; i++, stream += 8, data += 8) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
            stream[2] += data[2] * left;
            stream[3] += data[3] * right;
            stream[4] += data[4] * left;
            stream[5] += data[5] * right;
            stream[6] += data[6] * left;
            stream[7] += data[7] * right;
        }
        for (i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
        }
    }
}

#ifdef __SSE__
static void mix_float32_c1_sse(const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 8;
    const int leftover = mixframes % 8;
    ALsizei i;

    /* We can align this to 16 in one special case. */
    if ( ((((size_t)data) % 16) == 8) && ((((size_t)stream) % 16) == 0) && (mixframes >= 2) ) {
        stream[0] += data[0] * left;
        stream[1] += data[0] * right;
        stream[2] += data[1] * left;
        stream[3] += data[1] * right;
        mix_float32_c1_sse(panning, data + 2, stream + 4, mixframes - 2);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        /* unaligned, do scalar version. */
        mix_float32_c1_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (i = 0; i < unrolled; i++, data += 8, stream += 16) {
            /* We have 8 SSE registers, load 6 of them, have two for math (unrolled once). */
            {
                const __m128 vdataload1 = _mm_load_ps(data);
                const __m128 vdataload2 = _mm_load_ps(data+4);
                const __m128 vstream1 = _mm_load_ps(stream);
                const __m128 vstream2 = _mm_load_ps(stream+4);
                const __m128 vstream3 = _mm_load_ps(stream+8);
                const __m128 vstream4 = _mm_load_ps(stream+12);
                _mm_store_ps(stream, _mm_add_ps(vstream1, _mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(0, 0, 1, 1))));
                _mm_store_ps(stream+4, _mm_add_ps(vstream2, _mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(2, 2, 3, 3))));
                _mm_store_ps(stream+8, _mm_add_ps(vstream3, _mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(0, 0, 1, 1))));
                _mm_store_ps(stream+12, _mm_add_ps(vstream4, _mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(2, 2, 3, 3))));
            }
        }
        for (i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp;
            stream[1] += samp;
        }
    } else {
        const __m128 vleftright = { left, right, left, right };
        for (i = 0; i < unrolled; i++, data += 8, stream += 16) {
            /* We have 8 SSE registers, load 6 of them, have two for math (unrolled once). */
            const __m128 vdataload1 = _mm_load_ps(data);
            const __m128 vdataload2 = _mm_load_ps(data+4);
            const __m128 vstream1 = _mm_load_ps(stream);
            const __m128 vstream2 = _mm_load_ps(stream+4);
            const __m128 vstream3 = _mm_load_ps(stream+8);
            const __m128 vstream4 = _mm_load_ps(stream+12);
            _mm_store_ps(stream, _mm_add_ps(vstream1, _mm_mul_ps(_mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(0, 0, 1, 1)), vleftright)));
            _mm_store_ps(stream+4, _mm_add_ps(vstream2, _mm_mul_ps(_mm_shuffle_ps(vdataload1, vdataload1, _MM_SHUFFLE(2, 2, 3, 3)), vleftright)));
            _mm_store_ps(stream+8, _mm_add_ps(vstream3, _mm_mul_ps(_mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(0, 0, 1, 1)), vleftright)));
            _mm_store_ps(stream+12, _mm_add_ps(vstream4, _mm_mul_ps(_mm_shuffle_ps(vdataload2, vdataload2, _MM_SHUFFLE(2, 2, 3, 3)), vleftright)));
        }
        for (i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp * left;
            stream[1] += samp * right;
        }
    }
}

static void mix_float32_c2_sse(const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 4;
    const int leftover = mixframes % 4;
    ALsizei i;

    /* We can align this to 16 in one special case. */
    if ( ((((size_t)stream) % 16) == 8) && ((((size_t)data) % 16) == 8) && mixframes ) {
        stream[0] += data[0] * left;
        stream[1] += data[1] * right;
        mix_float32_c2_sse(panning, data + 2, stream + 2, mixframes - 1);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        /* unaligned, do scalar version. */
        mix_float32_c2_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (i = 0; i < unrolled; i++, data += 8, stream += 8) {
            const __m128 vdata1 = _mm_load_ps(data);
            const __m128 vdata2 = _mm_load_ps(data+4);
            const __m128 vstream1 = _mm_load_ps(stream);
            const __m128 vstream2 = _mm_load_ps(stream+4);
            _mm_store_ps(stream, _mm_add_ps(vstream1, vdata1));
            _mm_store_ps(stream+4, _mm_add_ps(vstream2, vdata2));
        }
        for (i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0];
            stream[1] += data[1];
        }
    } else {
        const __m128 vleftright = { left, right, left, right };
        for (i = 0; i < unrolled; i++, data += 8, stream += 8) {
            const __m128 vdata1 = _mm_load_ps(data);
            const __m128 vdata2 = _mm_load_ps(data+4);
            const __m128 vstream1 = _mm_load_ps(stream);
            const __m128 vstream2 = _mm_load_ps(stream+4);
            _mm_store_ps(stream, _mm_add_ps(vstream1, _mm_mul_ps(vdata1, vleftright)));
            _mm_store_ps(stream+4, _mm_add_ps(vstream2, _mm_mul_ps(vdata2, vleftright)));
        }
        for (i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
        }
    }
}
#endif

#ifdef __ARM_NEON__
static void mix_float32_c1_neon(const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 8;
    const int leftover = mixframes % 8;
    ALsizei i;

    /* We can align this to 16 in one special case. */
    if ( ((((size_t)data) % 16) == 8) && ((((size_t)stream) % 16) == 0) && (mixframes >= 2) ) {
        stream[0] += data[0] * left;
        stream[1] += data[0] * right;
        stream[2] += data[1] * left;
        stream[3] += data[1] * right;
        mix_float32_c1_neon(panning, data + 2, stream + 4, mixframes - 2);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        /* unaligned, do scalar version. */
        mix_float32_c1_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (i = 0; i < unrolled; i++, data += 8, stream += 16) {
            const float32x4_t vdataload1 = vld1q_f32(data);
            const float32x4_t vdataload2 = vld1q_f32(data+4);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            const float32x4x2_t vzipped1 = vzipq_f32(vdataload1, vdataload1);
            const float32x4x2_t vzipped2 = vzipq_f32(vdataload2, vdataload2);
            vst1q_f32(stream, vaddq_f32(vstream1, vzipped1.val[0]));
            vst1q_f32(stream+4, vaddq_f32(vstream2, vzipped1.val[1]));
            vst1q_f32(stream+8, vaddq_f32(vstream3, vzipped2.val[0]));
            vst1q_f32(stream+12, vaddq_f32(vstream4, vzipped2.val[1]));
        }
        for (i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp;
            stream[1] += samp;
        }
    } else {
        const float32x4_t vleftright = { left, right, left, right };
        for (i = 0; i < unrolled; i++, data += 8, stream += 16) {
            const float32x4_t vdataload1 = vld1q_f32(data);
            const float32x4_t vdataload2 = vld1q_f32(data+4);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            const float32x4x2_t vzipped1 = vzipq_f32(vdataload1, vdataload1);
            const float32x4x2_t vzipped2 = vzipq_f32(vdataload2, vdataload2);
            vst1q_f32(stream, vmlaq_f32(vstream1, vzipped1.val[0], vleftright));
            vst1q_f32(stream+4, vmlaq_f32(vstream2, vzipped1.val[1], vleftright));
            vst1q_f32(stream+8, vmlaq_f32(vstream3, vzipped2.val[0], vleftright));
            vst1q_f32(stream+12, vmlaq_f32(vstream4, vzipped2.val[1], vleftright));
        }
        for (i = 0; i < leftover; i++, stream += 2) {
            const float samp = *(data++);
            stream[0] += samp * left;
            stream[1] += samp * right;
        }
    }
}

static void mix_float32_c2_neon(const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    const int unrolled = mixframes / 8;
    const int leftover = mixframes % 8;
    ALsizei i;

    /* We can align this to 16 in one special case. */
    if ( ((((size_t)stream) % 16) == 8) && ((((size_t)data) % 16) == 8) && mixframes ) {
        stream[0] += data[0] * left;
        stream[1] += data[1] * right;
        mix_float32_c2_neon(panning, data + 2, stream + 2, mixframes - 1);
    } else if ( (((size_t)stream) % 16) || (((size_t)data) % 16) ) {
        /* unaligned, do scalar version. */
        mix_float32_c2_scalar(panning, data, stream, mixframes);
    } else if ((left == 1.0f) && (right == 1.0f)) {
        for (i = 0; i < unrolled; i++, data += 16, stream += 16) {
            const float32x4_t vdata1 = vld1q_f32(data);
            const float32x4_t vdata2 = vld1q_f32(data+4);
            const float32x4_t vdata3 = vld1q_f32(data+8);
            const float32x4_t vdata4 = vld1q_f32(data+12);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            vst1q_f32(stream, vaddq_f32(vstream1, vdata1));
            vst1q_f32(stream+4, vaddq_f32(vstream2, vdata2));
            vst1q_f32(stream+8, vaddq_f32(vstream3, vdata3));
            vst1q_f32(stream+12, vaddq_f32(vstream4, vdata4));
        }
        for (i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0];
            stream[1] += data[1];
        }
    } else {
        const float32x4_t vleftright = { left, right, left, right };
        for (i = 0; i < unrolled; i++, data += 16, stream += 16) {
            const float32x4_t vdata1 = vld1q_f32(data);
            const float32x4_t vdata2 = vld1q_f32(data+4);
            const float32x4_t vdata3 = vld1q_f32(data+8);
            const float32x4_t vdata4 = vld1q_f32(data+12);
            const float32x4_t vstream1 = vld1q_f32(stream);
            const float32x4_t vstream2 = vld1q_f32(stream+4);
            const float32x4_t vstream3 = vld1q_f32(stream+8);
            const float32x4_t vstream4 = vld1q_f32(stream+12);
            vst1q_f32(stream, vmlaq_f32(vstream1, vdata1, vleftright));
            vst1q_f32(stream+4, vmlaq_f32(vstream2, vdata2, vleftright));
            vst1q_f32(stream+8, vmlaq_f32(vstream3, vdata3, vleftright));
            vst1q_f32(stream+12, vmlaq_f32(vstream4, vdata4, vleftright));
        }
        for (i = 0; i < leftover; i++, stream += 2, data += 2) {
            stream[0] += data[0] * left;
            stream[1] += data[1] * right;
        }
    }
}
#endif


static void mix_buffer(const ALbuffer *buffer, const ALfloat * restrict panning, const float * restrict data, float * restrict stream, const ALsizei mixframes)
{
    const ALfloat left = panning[0];
    const ALfloat right = panning[1];
    FIXME("currently expects output to be stereo");
    if ((left != 0.0f) || (right != 0.0f)) {  /* don't bother mixing in silence. */
        if (buffer->channels == 1) {
            #ifdef __SSE__
            if (has_sse) { mix_float32_c1_sse(panning, data, stream, mixframes); } else
            #elif defined(__ARM_NEON__)
            if (has_neon) { mix_float32_c1_neon(panning, data, stream, mixframes); } else
            #endif
            {
            #if NEED_SCALAR_FALLBACK
            mix_float32_c1_scalar(panning, data, stream, mixframes);
            #else
            SDL_assert(!"uhoh, we didn't compile in enough mixers!");
            #endif
            }
        } else {
            SDL_assert(buffer->channels == 2);
            #ifdef __SSE__
            if (has_sse) { mix_float32_c2_sse(panning, data, stream, mixframes); } else
            #elif defined(__ARM_NEON__)
            if (has_neon) { mix_float32_c2_neon(panning, data, stream, mixframes); } else
            #endif
            {
            #if NEED_SCALAR_FALLBACK
            mix_float32_c2_scalar(panning, data, stream, mixframes);
            #else
            SDL_assert(!"uhoh, we didn't compile in enough mixers!");
            #endif
            }
        }
    }
}

static ALboolean mix_source_buffer(ALCcontext *ctx, ALsource *src, BufferQueueItem *queue, float **stream, int *len)
{
    const ALbuffer *buffer = queue ? queue->buffer : NULL;
    ALboolean processed = AL_TRUE;

    /* you can legally queue or set a NULL buffer. */
    if (buffer && buffer->data && (buffer->len > 0)) {
        const float *data = buffer->data + (src->offset / sizeof (float));
        const int bufferframesize = (int) (buffer->channels * sizeof (float));
        const int deviceframesize = ctx->device->framesize;
        const int framesneeded = *len / deviceframesize;
        const float pitch = src->pitch;

        SDL_assert(src->offset < buffer->len);

        /*
        SRC_SINC_BEST_QUALITY
        SRC_SINC_MEDIUM_QUALITY
        SRC_SINC_FASTEST
        SRC_ZERO_ORDER_HOLD
        SRC_LINEAR
        */
        const int converter = SRC_LINEAR;
        int src_error = 0;
        if ((pitch != 1.0f || ctx->device->frequency != buffer->frequency) && src->src_state == NULL) {
            src->src_state = src_new(converter, buffer->channels, &src_error);
            if (src->src_state == NULL)
                fprintf(stderr, "src_new error: %s\n", src_strerror(src_error));
        }
        else if (pitch == 1.0f && ctx->device->frequency == buffer->frequency && src->src_state != NULL) {
            src_delete(src->src_state);
            src->src_state = NULL;
        }
        else if (src->src_state != NULL && src_get_channels(src->src_state) != buffer->channels) {
            src_delete(src->src_state);
            src->src_state = src_new(converter, buffer->channels, &src_error);
            if (src->src_state == NULL)
                fprintf(stderr, "src_new error: %s\n", src_strerror(src_error));
        }
        if (src->src_state) {  /* resampling? */
            int used_frame = 0;
            do {
                SRC_DATA src_data = {};
                src_data.input_frames = (buffer->len - src->offset) / buffer->channels / sizeof (float);
                src_data.output_frames = framesneeded - used_frame;
                if (src_data.output_frames <= 0 || src->offset >= buffer->len)
                    break;
                src_data.data_in = data;
                float *data_out = (float *) alloca((framesneeded - used_frame) * buffer->channels * sizeof (float));
                src_data.data_out = data_out;
                src_data.src_ratio = (double) ctx->device->frequency / (double) buffer->frequency / pitch;
                int ret = src_process(src->src_state, &src_data);
                if (ret != 0) {
                    fprintf(stderr, "src_process error: %s\n", src_strerror(ret));
                    break;
                }
                mix_buffer(buffer, src->panning, src_data.data_out, *stream, (ALsizei)src_data.output_frames_gen);
                *len -= src_data.output_frames_gen * deviceframesize;
                *stream += src_data.output_frames_gen * ctx->device->channels;
                src->offset += src_data.input_frames_used * buffer->channels * sizeof (float);
                used_frame += src_data.output_frames_gen;
                data += src_data.input_frames_used * buffer->channels;
            } while (*len > 0);
        } else {
            const int framesavail = (buffer->len - src->offset) / bufferframesize;
            const int mixframes = SDL_min(framesneeded, framesavail);
            mix_buffer(buffer, src->panning, data, *stream, mixframes);
            src->offset += mixframes * bufferframesize;
            *len -= mixframes * deviceframesize;
            *stream += mixframes * ctx->device->channels;
        }

        SDL_assert(src->offset <= buffer->len);

        processed = src->offset >= buffer->len;
        if (processed) {
            FIXME("does the offset have to represent the whole queue or just the current buffer?");
            src->offset = 0;
        }
    }

    return processed;
}

static ALCboolean mix_source_buffer_queue(ALCcontext *ctx, ALsource *src, BufferQueueItem *queue, float *stream, int len)
{
    ALCboolean keep = ALC_TRUE;

    while ((len > 0) && (mix_source_buffer(ctx, src, queue, &stream, &len))) {
        /* Finished this buffer! */
        BufferQueueItem *item = queue;
        BufferQueueItem *next = queue ? queue->next : NULL;
        void *ptr;

        if (queue) {
            queue->next = NULL;
            queue = next;
        }

        SDL_assert((src->type == AL_STATIC) || (src->type == AL_STREAMING));
        if (src->type == AL_STREAMING) {  /* mark buffer processed. */
            SDL_assert(item == src->buffer_queue.head);
            FIXME("bubble out all these NULL checks");  /* these are only here because we check for looping/stopping in this loop, but we really shouldn't enter this loop at all if queue==NULL. */
            if (item != NULL) {
                src->buffer_queue.head = next;
                if (!next) {
                    src->buffer_queue.tail = NULL;
                }
                SDL_AtomicAdd(&src->buffer_queue.num_items, -1);

                /* Move it to the processed queue for alSourceUnqueueBuffers() to pick up. */
                do {
                    ptr = SDL_AtomicGetPtr(&src->buffer_queue_processed.just_queued);
                    SDL_AtomicSetPtr(&item->next, ptr);
                } while (!SDL_AtomicCASPtr(&src->buffer_queue_processed.just_queued, ptr, item));

                SDL_AtomicAdd(&src->buffer_queue_processed.num_items, 1);
            }
        }

        if (queue == NULL) {  /* nothing else to play? */
            if (src->looping) {
                FIXME("looping is supposed to move to AL_INITIAL then immediately to AL_PLAYING, but I'm not sure what side effect this is meant to trigger");
                if (src->type == AL_STREAMING) {
                    FIXME("what does looping do with the AL_STREAMING state?");
                }
            } else {
                SDL_AtomicSet(&src->state, AL_STOPPED);
                keep = ALC_FALSE;
            }
            break;  /* nothing else to mix here, so stop. */
        }
    }

    return keep;
}

/* All the 3D math here is way overcommented because I HAVE NO IDEA WHAT I'M
   DOING and had to research the hell out of what are probably pretty simple
   concepts. Pay attention in math class, kids. */

/* The scalar versions have explanitory comments and links. The SIMD versions don't. */

/* calculates cross product. https://en.wikipedia.org/wiki/Cross_product
    Basically takes two vectors and gives you a vector that's perpendicular
    to both.
*/
#if NEED_SCALAR_FALLBACK
static void xyzzy(ALfloat *v, const ALfloat *a, const ALfloat *b)
{
    v[0] = (a[1] * b[2]) - (a[2] * b[1]);
    v[1] = (a[2] * b[0]) - (a[0] * b[2]);
    v[2] = (a[0] * b[1]) - (a[1] * b[0]);
}

/* calculate dot product (multiply each element of two vectors, sum them) */
static ALfloat dotproduct(const ALfloat *a, const ALfloat *b)
{
    return (a[0] * b[0]) + (a[1] * b[1]) + (a[2] * b[2]);
}

/* calculate distance ("magnitude") in 3D space:
    https://math.stackexchange.com/questions/42640/calculate-distance-in-3d-space
    assumes vector starts at (0,0,0). */
static ALfloat magnitude(const ALfloat *v)
{
    /* technically, the inital part on this is just a dot product of itself. */
    return SDL_sqrtf((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

/* https://www.khanacademy.org/computing/computer-programming/programming-natural-simulations/programming-vectors/a/vector-magnitude-normalization */
static void normalize(ALfloat *v)
{
    const ALfloat mag = magnitude(v);
    if (mag == 0.0f) {
        SDL_memset(v, '\0', sizeof (*v) * 3);
    } else {
        v[0] /= mag;
        v[1] /= mag;
        v[2] /= mag;
    }
}
#endif

#ifdef __SSE__
static __m128 xyzzy_sse(const __m128 a, const __m128 b)
{
    /* http://fastcpp.blogspot.com/2011/04/vector-cross-product-using-sse-code.html
        this is the "three shuffle" version in the comments, plus the variables swapped around for handedness in the later comment. */
    const __m128 v = _mm_sub_ps(
        _mm_mul_ps(a, _mm_shuffle_ps(b, b, _MM_SHUFFLE(3, 0, 2, 1))),
        _mm_mul_ps(b, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 0, 2, 1)))
    );
    return _mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 0, 2, 1));
}

static ALfloat dotproduct_sse(const __m128 a, const __m128 b)
{
    const __m128 prod = _mm_mul_ps(a, b);
    const __m128 sum1 = _mm_add_ps(prod, _mm_shuffle_ps(prod, prod, _MM_SHUFFLE(1, 0, 3, 2)));
    const __m128 sum2 = _mm_add_ps(sum1, _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(2, 2, 0, 0)));
    FIXME("this can use _mm_hadd_ps in SSE3, or _mm_dp_ps in SSE4.1");
    return _mm_cvtss_f32(_mm_shuffle_ps(sum2, sum2, _MM_SHUFFLE(3, 3, 3, 3)));
}

static ALfloat magnitude_sse(const __m128 v)
{
    return SDL_sqrtf(dotproduct_sse(v, v));
}

static __m128 normalize_sse(const __m128 v)
{
    const ALfloat mag = magnitude_sse(v);
    if (mag == 0.0f) {
        return _mm_setzero_ps();
    }
    return _mm_div_ps(v, _mm_set_ps1(mag));
}
#endif

#ifdef __ARM_NEON__
static float32x4_t xyzzy_neon(const float32x4_t a, const float32x4_t b)
{
    const float32x4_t shuf_a = { a[1], a[2], a[0], a[3] };
    const float32x4_t shuf_b = { b[1], b[2], b[0], b[3] };
    const float32x4_t v = vsubq_f32(vmulq_f32(a, shuf_b), vmulq_f32(b, shuf_a));
    const float32x4_t retval = { v[1], v[2], v[0], v[3] };
    FIXME("need a better permute");
    return retval;
}

static ALfloat dotproduct_neon(const float32x4_t a, const float32x4_t b)
{
    const float32x4_t prod = vmulq_f32(a, b);
    const float32x4_t sum1 = vaddq_f32(prod, vrev64q_f32(prod));
    const float32x4_t sum2 = vaddq_f32(sum1, vcombine_f32(vget_high_f32(sum1), vget_low_f32(sum1)));
    return sum2[3];
}

static ALfloat magnitude_neon(const float32x4_t v)
{
    return SDL_sqrtf(dotproduct_neon(v, v));
}

static float32x4_t normalize_neon(const float32x4_t v)
{
    const ALfloat mag = magnitude_neon(v);
    if (mag == 0.0f) {
        return vdupq_n_f32(0.0f);
    }
    return vmulq_f32(v, vdupq_n_f32(1.0f / mag));
}
#endif



/* Get the sin(angle) and cos(angle) at the same time. Ideally, with one
   instruction, like what is offered on the x86.
   angle is in radians, not degrees. */
static void calculate_sincos(const ALfloat angle, ALfloat *_sin, ALfloat *_cos)
{
    *_sin = SDL_sinf(angle);
    *_cos = SDL_cosf(angle);
}

static ALfloat calculate_distance_attenuation(const ALCcontext *ctx, const ALsource *src, ALfloat distance)
{
    /* AL SPEC: "With all the distance models, if the formula can not be
       evaluated then the source will not be attenuated. For example, if a
       linear model is being used with AL_REFERENCE_DISTANCE equal to
       AL_MAX_DISTANCE, then the gain equation will have a divide-by-zero
       error in it. In this case, there is no attenuation for that source." */
    FIXME("check divisions by zero");

    switch (ctx->distance_model) {
        case AL_INVERSE_DISTANCE_CLAMPED:
            distance = SDL_min(SDL_max(distance, src->reference_distance), src->max_distance);
            /* fallthrough */
        case AL_INVERSE_DISTANCE:
            /* AL SPEC: "gain = AL_REFERENCE_DISTANCE / (AL_REFERENCE_DISTANCE + AL_ROLLOFF_FACTOR * (distance - AL_REFERENCE_DISTANCE))" */
            return src->reference_distance / (src->reference_distance + src->rolloff_factor * (distance - src->reference_distance));

        case AL_LINEAR_DISTANCE_CLAMPED:
            distance = SDL_max(distance, src->reference_distance);
            /* fallthrough */
        case AL_LINEAR_DISTANCE:
            /* AL SPEC: "distance = min(distance, AL_MAX_DISTANCE) // avoid negative gain
                         gain = (1 - AL_ROLLOFF_FACTOR * (distance - AL_REFERENCE_DISTANCE) / (AL_MAX_DISTANCE - AL_REFERENCE_DISTANCE))" */
            return 1.0f - src->rolloff_factor * (SDL_min(distance, src->max_distance) - src->reference_distance) / (src->max_distance - src->reference_distance);

        case AL_EXPONENT_DISTANCE_CLAMPED:
            distance = SDL_min(SDL_max(distance, src->reference_distance), src->max_distance);
            /* fallthrough */
        case AL_EXPONENT_DISTANCE:
            /* AL SPEC: "gain = (distance / AL_REFERENCE_DISTANCE) ^ (- AL_ROLLOFF_FACTOR)" */
            return SDL_powf(distance / src->reference_distance, -src->rolloff_factor);

        default: break;
    }

    SDL_assert(!"Unexpected distance model");
    return 1.0f;
}

static void calculate_channel_gains(const ALCcontext *ctx, const ALsource *src, float *gains)
{
    /* rolloff==0.0f makes all distance models result in 1.0f,
       and we never spatialize non-mono sources, per the AL spec. */
    const ALboolean spatialize = (ctx->distance_model != AL_NONE) &&
                                 (src->queue_channels == 1) &&
                                 (src->rolloff_factor != 0.0f);

    const ALfloat *at = &ctx->listener.orientation[0];
    const ALfloat *up = &ctx->listener.orientation[4];

    ALfloat distance;
    ALfloat gain;
    ALfloat radians;

    #ifdef __SSE__
    __m128 position_sse;
    #elif defined(__ARM_NEON__)
    float32x4_t position_neon = vdupq_n_f32(0.0f);
    #endif

    #if NEED_SCALAR_FALLBACK
    ALfloat position[3];
    #endif

    /* this goes through the steps the AL spec dictates for gain and distance attenuation... */

    if (!spatialize) {
        /* simpler path through the same AL spec details if not spatializing. */
        gain = SDL_min(SDL_max(src->gain, src->min_gain), src->max_gain) * ctx->listener.gain;
        gains[0] = gains[1] = gain;  /* no spatialization, but AL_GAIN (etc) is still applied. */
        return;
    }

    #ifdef __SSE__
    if (has_sse) {
        position_sse = _mm_load_ps(src->position);
        if (!src->source_relative) {
            position_sse = _mm_sub_ps(position_sse, _mm_load_ps(ctx->listener.position));
        }
        distance = magnitude_sse(position_sse);
    } else
    #elif defined(__ARM_NEON__)
    if (has_neon) {
        position_neon = vld1q_f32(src->position);
        if (!src->source_relative) {
            position_neon = vsubq_f32(position_neon, vld1q_f32(ctx->listener.position));
        }
        distance = magnitude_neon(position_neon);
    } else
    #endif

    {
    #if NEED_SCALAR_FALLBACK
    SDL_memcpy(position, src->position, sizeof (position));
    /* if values aren't source-relative, then convert it to be so. */
    if (!src->source_relative) {
        position[0] -= ctx->listener.position[0];
        position[1] -= ctx->listener.position[1];
        position[2] -= ctx->listener.position[2];
    }
    distance = magnitude(position);
    #endif
    }

    /* AL SPEC: ""1. Distance attenuation is calculated first, including
       minimum (AL_REFERENCE_DISTANCE) and maximum (AL_MAX_DISTANCE)
       thresholds." */
    gain = calculate_distance_attenuation(ctx, src, distance);

    /* AL SPEC: "2. The result is then multiplied by source gain (AL_GAIN)." */
    gain *= src->gain;

    /* AL SPEC: "3. If the source is directional (AL_CONE_INNER_ANGLE less
       than AL_CONE_OUTER_ANGLE), an angle-dependent attenuation is calculated
       depending on AL_CONE_OUTER_GAIN, and multiplied with the distance
       dependent attenuation. The resulting attenuation factor for the given
       angle and distance between listener and source is multiplied with
       source AL_GAIN." */
    if (src->cone_inner_angle < src->cone_outer_angle) {
        FIXME("directional sources");
    }

    /* AL SPEC: "4. The effective gain computed this way is compared against
       AL_MIN_GAIN and AL_MAX_GAIN thresholds." */
    gain = SDL_min(SDL_max(gain, src->min_gain), src->max_gain);

    /* AL SPEC: "5. The result is guaranteed to be clamped to [AL_MIN_GAIN,
       AL_MAX_GAIN], and subsequently multiplied by listener gain which serves
       as an overall volume control. The implementation is free to clamp
       listener gain if necessary due to hardware or implementation
       constraints." */
    gain *= ctx->listener.gain;

    /* now figure out positioning. Since we're aiming for stereo, we just
       need a simple panning effect. We're going to do what's called
       "constant power panning," as explained...

       https://dsp.stackexchange.com/questions/21691/algorithm-to-pan-audio

       Naturally, we'll need to know the angle between where our listener
       is facing and where the source is to make that work...

       https://www.youtube.com/watch?v=S_568VZWFJo

       ...but to do that, we need to rotate so we have the correct side of
       the listener, which isn't just a point in space, but has a definite
       direction it is facing. More or less, this is what gluLookAt deals
       with...

       http://www.songho.ca/opengl/gl_camera.html

       ...although I messed with the algorithm until it did what I wanted.

       XYZZY!! https://en.wikipedia.org/wiki/Cross_product#Mnemonic
    */

    #ifdef __SSE__ /* (the math is explained in the scalar version.) */
    if (has_sse) {
        const __m128 at_sse = _mm_load_ps(at);
        const __m128 U_sse = normalize_sse(xyzzy_sse(at_sse, _mm_load_ps(up)));
        const __m128 V_sse = xyzzy_sse(at_sse, U_sse);
        const __m128 N_sse = normalize_sse(at_sse);
        const __m128 rotated_sse = {
            dotproduct_sse(position_sse, U_sse),
            -dotproduct_sse(position_sse, V_sse),
            -dotproduct_sse(position_sse, N_sse),
            0.0f
        };

        const ALfloat mags = magnitude_sse(at_sse) * magnitude_sse(rotated_sse);
        radians = (mags == 0.0f) ? 0.0f : SDL_acosf(dotproduct_sse(at_sse, rotated_sse) / mags);
        if (_mm_comilt_ss(rotated_sse, _mm_setzero_ps())) {
            radians = -radians;
        }
    } else
    #endif

    #ifdef __ARM_NEON__  /* (the math is explained in the scalar version.) */
    if (has_neon) {
        const float32x4_t at_neon = vld1q_f32(at);
        const float32x4_t U_neon = normalize_neon(xyzzy_neon(at_neon, vld1q_f32(up)));
        const float32x4_t V_neon = xyzzy_neon(at_neon, U_neon);
        const float32x4_t N_neon = normalize_neon(at_neon);
        const float32x4_t rotated_neon = {
            dotproduct_neon(position_neon, U_neon),
            -dotproduct_neon(position_neon, V_neon),
            -dotproduct_neon(position_neon, N_neon),
            0.0f
        };

        const ALfloat mags = magnitude_neon(at_neon) * magnitude_neon(rotated_neon);
        radians = (mags == 0.0f) ? 0.0f : SDL_acosf(dotproduct_neon(at_neon, rotated_neon) / mags);
        if (rotated_neon[0] < 0.0f) {
            radians = -radians;
        }
    } else
    #endif

    {
    #if NEED_SCALAR_FALLBACK
        ALfloat U[3];
        ALfloat V[3];
        ALfloat N[3];
        ALfloat rotated[3];
        ALfloat mags;

        xyzzy(U, at, up);
        normalize(U);
        xyzzy(V, at, U);
        SDL_memcpy(N, at, sizeof (N));
        normalize(N);

        /* we don't need the bottom row of the gluLookAt matrix, since we don't
           translate. (Matrix * Vector) is just filling in each element of the
           output vector with the dot product of a row of the matrix and the
           vector. I made some of these negative to make it work for my purposes,
           but that's not what GLU does here.

           (This says gluLookAt is left-handed, so maybe that's part of it?)
            https://stackoverflow.com/questions/25933581/how-u-v-n-camera-coordinate-system-explained-with-opengl
         */
        rotated[0] = dotproduct(position, U);
        rotated[1] = -dotproduct(position, V);
        rotated[2] = -dotproduct(position, N);

        /* At this point, we have rotated vector and we can calculate the angle
           from 0 (directly in front of where the listener is facing) to 180
           degrees (directly behind) ... */

        mags = magnitude(at) * magnitude(rotated);
        radians = (mags == 0.0f) ? 0.0f : SDL_acosf(dotproduct(at, rotated) / mags);
        /* and we already have what we need to decide if those degrees are on the
           listener's left or right...
           https://gamedev.stackexchange.com/questions/43897/determining-if-something-is-on-the-right-or-left-side-of-an-object
           ...we already did this dot product: it's in rotated[0]. */

        /* make it negative to the left, positive to the right. */
        if (rotated[0] < 0.0f) {
            radians = -radians;
        }
    #endif
    }

    /* here comes the Constant Power Panning magic... */
    #define SQRT2_DIV2 0.7071067812f  /* sqrt(2.0) / 2.0 ... */

    /* this might be a terrible idea, which is totally my own doing here,
      but here you go: Constant Power Panning only works from -45 to 45
      degrees in front of the listener. So we split this into 4 quadrants.
      - from -45 to 45: standard panning.
      - from 45 to 135: pan full right.
      - from 135 to 225: flip angle so it works like standard panning.
      - from 225 to -45: pan full left. */

    #define RADIANS_45_DEGREES 0.7853981634f
    #define RADIANS_135_DEGREES 2.3561944902f
    if ((radians >= -RADIANS_45_DEGREES) && (radians <= RADIANS_45_DEGREES)) {
        ALfloat sine, cosine;
        calculate_sincos(radians, &sine, &cosine);
        gains[0] = (SQRT2_DIV2 * (cosine - sine));
        gains[1] = (SQRT2_DIV2 * (cosine + sine));
    } else if ((radians >= RADIANS_45_DEGREES) && (radians <= RADIANS_135_DEGREES)) {
        gains[0] = 0.0f;
        gains[1] = 1.0f;
    } else if ((radians >= -RADIANS_135_DEGREES) && (radians <= -RADIANS_45_DEGREES)) {
        gains[0] = 1.0f;
        gains[1] = 0.0f;
    } else if (radians < 0.0f) {  /* back left */
        ALfloat sine, cosine;
        calculate_sincos((ALfloat) -(radians + M_PI), &sine, &cosine);
        gains[0] = (SQRT2_DIV2 * (cosine - sine));
        gains[1] = (SQRT2_DIV2 * (cosine + sine));
    } else { /* back right */
        ALfloat sine, cosine;
        calculate_sincos((ALfloat) -(radians - M_PI), &sine, &cosine);
        gains[0] = (SQRT2_DIV2 * (cosine - sine));
        gains[1] = (SQRT2_DIV2 * (cosine + sine));
    }

    /* apply distance attenuation and gain to positioning. */
    gains[0] *= gain;
    gains[1] *= gain;
}


static ALCboolean mix_source(ALCcontext *ctx, ALsource *src, float *stream, int len, const ALboolean force_recalc)
{
    ALCboolean keep;

    keep = (SDL_AtomicGet(&src->state) == AL_PLAYING);
    if (keep) {
        SDL_assert(src->allocated);
        if (src->recalc || force_recalc) {
            SDL_MemoryBarrierAcquire();
            src->recalc = AL_FALSE;
            calculate_channel_gains(ctx, src, src->panning);
        }
        if (src->type == AL_STATIC) {
            BufferQueueItem fakequeue = { src->buffer, NULL };
            keep = mix_source_buffer_queue(ctx, src, &fakequeue, stream, len);
        } else if (src->type == AL_STREAMING) {
            obtain_newly_queued_buffers(&src->buffer_queue);
            keep = mix_source_buffer_queue(ctx, src, src->buffer_queue.head, stream, len);
        } else if (src->type == AL_UNDETERMINED) {
            keep = ALC_FALSE;  /* this has AL_BUFFER set to 0; just dump it. */
        } else {
            SDL_assert(!"unknown source type");
        }
    }

    return keep;
}

/* move new play requests over to the mixer thread. */
static void migrate_playlist_requests(ALCcontext *ctx)
{
    SourcePlayTodo *todo;
    SourcePlayTodo *todoend;
    SourcePlayTodo *i;

    do {  /* take the todo list atomically, now we own it. */
        todo = (SourcePlayTodo *) ctx->playlist_todo;
    } while (!SDL_AtomicCASPtr(&ctx->playlist_todo, todo, NULL));

    if (!todo) {
        return;  /* nothing new. */
    }

    todoend = todo;

    /* ctx->playlist and ALsource->playlist_next are only every touched
       by the mixer thread, and source pointers live until context destruction. */
    for (i = todo; i != NULL; i = i->next) {
        todoend = i;
        if ((i->source != ctx->playlist_tail) && (!i->source->playlist_next)) {
            i->source->playlist_next = ctx->playlist;
            if (!ctx->playlist) {
                ctx->playlist_tail = i->source;
            }
            ctx->playlist = i->source;
        }
    }

    /* put these objects back in the pool for reuse */
    do {
        todoend->next = i = (SourcePlayTodo *) ctx->device->playback.source_todo_pool;
    } while (!SDL_AtomicCASPtr(&ctx->device->playback.source_todo_pool, i, todo));
}

static void mix_context(ALCcontext *ctx, float *stream, int len)
{
    const ALboolean force_recalc = ctx->recalc;
    ALsource *next = NULL;
    ALsource *prev = NULL;
    ALsource *i;

    if (force_recalc) {
        SDL_MemoryBarrierAcquire();
        ctx->recalc = AL_FALSE;
    }

    migrate_playlist_requests(ctx);

    for (i = ctx->playlist; i != NULL; i = next) {
        next = i->playlist_next;  /* save this to a local in case we leave the list. */

        SDL_LockMutex(ctx->source_lock);
        if (!mix_source(ctx, i, stream, len, force_recalc)) {
            /* take it out of the playlist. It wasn't actually playing or it just finished. */
            i->playlist_next = NULL;
            if (next == NULL) {
                SDL_assert(i == ctx->playlist_tail);
                ctx->playlist_tail = prev;
            }
            if (prev) {
                prev->playlist_next = next;
            } else {
                SDL_assert(i == ctx->playlist);
                ctx->playlist = next;
            }
            SDL_AtomicSet(&i->mixer_accessible, 0);
        } else {
            prev = i;
        }
        SDL_UnlockMutex(ctx->source_lock);
    }
}

/* Disconnected devices move all PLAYING sources to STOPPED, making their buffer queues processed. */
static void mix_disconnected_context(ALCcontext *ctx)
{
    ALsource *next = NULL;
    ALsource *i;

    migrate_playlist_requests(ctx);

    for (i = ctx->playlist; i != NULL; i = next) {
        next = i->playlist_next;

        SDL_LockMutex(ctx->source_lock);
        /* remove from playlist; all playing things got stopped, paused/initial/stopped shouldn't be listed. */
        if (SDL_AtomicGet(&i->state) == AL_PLAYING) {
            SDL_assert(i->allocated);
            SDL_AtomicSet(&i->state, AL_STOPPED);
            source_mark_all_buffers_processed(i);
        }

        i->playlist_next = NULL;
        SDL_AtomicSet(&i->mixer_accessible, 0);
        SDL_UnlockMutex(ctx->source_lock);
    }
    ctx->playlist = NULL;
    ctx->playlist_tail = NULL;
}

/* We process all unsuspended ALC contexts during this call, mixing their
   output to (stream). SDL then plays this mixed audio to the hardware. */
static void SDLCALL playback_device_callback(void *userdata, Uint8 *stream, int len)
{
    ALCdevice *device = (ALCdevice *) userdata;
    ALCcontext *ctx;
    ALCboolean connected = ALC_FALSE;

    SDL_memset(stream, '\0', len);

    if (SDL_AtomicGet(&device->connected)) {
        if (SDL_GetAudioDeviceStatus(device->sdldevice) == SDL_AUDIO_STOPPED) {
            SDL_AtomicSet(&device->connected, ALC_FALSE);
        } else {
            connected = ALC_TRUE;
        }
    }

    for (ctx = device->playback.contexts; ctx != NULL; ctx = ctx->next) {
        if (SDL_AtomicGet(&ctx->processing)) {
            if (connected) {
                mix_context(ctx, (float *) stream, len);
            } else {
                mix_disconnected_context(ctx);
            }
        }
    }
}

static ALCcontext *_alcCreateContext(ALCdevice *device, const ALCint* attrlist)
{
    ALCcontext *retval = NULL;
    ALCsizei attrcount = 0;
    ALCint freq = 48000;
    ALCboolean sync = ALC_FALSE;
    ALCint refresh = 100;
    /* we don't care about ALC_MONO_SOURCES or ALC_STEREO_SOURCES as we have no hardware limitation. */

    if (!device) {
        set_alc_error(NULL, ALC_INVALID_DEVICE);
        return NULL;
    }

    if (!SDL_AtomicGet(&device->connected)) {
        set_alc_error(device, ALC_INVALID_DEVICE);
        return NULL;
    }

    if (attrlist != NULL) {
        ALCint attr;
        while ((attr = attrlist[attrcount++]) != 0) {
            switch (attr) {
                case ALC_FREQUENCY: freq = attrlist[attrcount++]; break;
                case ALC_REFRESH: refresh = attrlist[attrcount++]; break;
                case ALC_SYNC: sync = (attrlist[attrcount++] ? ALC_TRUE : ALC_FALSE); break;
                default: FIXME("fail for unknown attributes?"); break;
            }
        }
    }

    FIXME("use these variables at some point"); (void) refresh; (void) sync;

    retval = (ALCcontext *) calloc_simd_aligned(sizeof (ALCcontext));
    if (!retval) {
        set_alc_error(device, ALC_OUT_OF_MEMORY);
        return NULL;
    }

    /* Make sure everything that wants to use SIMD is aligned for it. */
    SDL_assert( (((size_t) &retval->listener.position[0]) % 16) == 0 );
    SDL_assert( (((size_t) &retval->listener.orientation[0]) % 16) == 0 );
    SDL_assert( (((size_t) &retval->listener.velocity[0]) % 16) == 0 );

    retval->source_lock = SDL_CreateMutex();
    if (!retval->source_lock) {
        set_alc_error(device, ALC_OUT_OF_MEMORY);
        free_simd_aligned(retval);
        return NULL;
    }

    retval->attributes = (ALCint *) SDL_malloc(attrcount * sizeof (ALCint));
    if (!retval->attributes) {
        set_alc_error(device, ALC_OUT_OF_MEMORY);
        SDL_DestroyMutex(retval->source_lock);
        free_simd_aligned(retval);
        return NULL;
    }
    SDL_memcpy(retval->attributes, attrlist, attrcount * sizeof (ALCint));
    retval->attributes_count = attrcount;

    if (!device->sdldevice) {
        SDL_AudioSpec desired;
        const char *devicename = device->name;

        if (SDL_strcmp(devicename, DEFAULT_PLAYBACK_DEVICE) == 0) {
            devicename = NULL;  /* tell SDL we want the best default */
        }

        /* we always want to work in float32, to keep our work simple and
           let us use SIMD, and we'll let SDL convert when feeding the device. */
        SDL_zero(desired);
        desired.freq = freq;
        desired.format = AUDIO_F32SYS;
        desired.channels = 2;  FIXME("don't force channels?");
        desired.samples = 1024;  FIXME("base this on refresh");
        desired.callback = playback_device_callback;
        desired.userdata = device;
        device->sdldevice = SDL_OpenAudioDevice(devicename, 0, &desired, NULL, 0);
        if (!device->sdldevice) {
            SDL_DestroyMutex(retval->source_lock);
            SDL_free(retval->attributes);
            free_simd_aligned(retval);
            FIXME("What error do you set for this?");
            return NULL;
        }
        device->channels = 2;
        device->frequency = freq;
        device->framesize = sizeof (float) * device->channels;
        SDL_PauseAudioDevice(device->sdldevice, 0);
    }

    retval->distance_model = AL_INVERSE_DISTANCE_CLAMPED;
    retval->doppler_factor = 1.0f;
    retval->doppler_velocity = 1.0f;
    retval->speed_of_sound = 343.3f;
    retval->listener.gain = 1.0f;
    retval->listener.orientation[2] = -1.0f;
    retval->listener.orientation[5] = 1.0f;
    retval->device = device;
    context_needs_recalc(retval);
    SDL_AtomicSet(&retval->processing, 1);  /* contexts default to processing */

    SDL_LockAudioDevice(device->sdldevice);
    if (device->playback.contexts != NULL) {
        SDL_assert(device->playback.contexts->prev == NULL);
        device->playback.contexts->prev = retval;
    }
    retval->next = device->playback.contexts;
    device->playback.contexts = retval;
    SDL_UnlockAudioDevice(device->sdldevice);

    return retval;
}
ENTRYPOINT(ALCcontext *,alcCreateContext,(ALCdevice *device, const ALCint* attrlist),(device,attrlist))


static SDL_INLINE ALCcontext *get_current_context(void)
{
    return (ALCcontext *) SDL_AtomicGetPtr(&current_context);
}

/* no api lock; it just sets an atomic pointer at the moment */
ALCboolean alcMakeContextCurrent(ALCcontext *ctx)
{
    SDL_AtomicSetPtr(&current_context, ctx);
    FIXME("any reason this might return ALC_FALSE?");
    return ALC_TRUE;
}

static void _alcProcessContext(ALCcontext *ctx)
{
    if (!ctx) {
        set_alc_error(NULL, ALC_INVALID_CONTEXT);
        return;
    }

    SDL_assert(!ctx->device->iscapture);
    SDL_AtomicSet(&ctx->processing, 1);
}
ENTRYPOINTVOID(alcProcessContext,(ALCcontext *ctx),(ctx))

static void _alcSuspendContext(ALCcontext *ctx)
{
    if (!ctx) {
        set_alc_error(NULL, ALC_INVALID_CONTEXT);
    } else {
        SDL_assert(!ctx->device->iscapture);
        SDL_AtomicSet(&ctx->processing, 0);
    }
}
ENTRYPOINTVOID(alcSuspendContext,(ALCcontext *ctx),(ctx))

static void _alcDestroyContext(ALCcontext *ctx)
{
    ALsizei blocki;

    FIXME("Should NULL context be an error?");
    if (!ctx) return;

    /* The spec says it's illegal to delete the current context. */
    if (get_current_context() == ctx) {
        set_alc_error(ctx->device, ALC_INVALID_CONTEXT);
        return;
    }

    /* do this first in case the mixer is running _right now_. */
    SDL_AtomicSet(&ctx->processing, 0);

    SDL_LockAudioDevice(ctx->device->sdldevice);
    if (ctx->prev) {
        ctx->prev->next = ctx->next;
    } else {
        SDL_assert(ctx == ctx->device->playback.contexts);
        ctx->device->playback.contexts = ctx->next;
    }
    if (ctx->next) {
        ctx->next->prev = ctx->prev;
    }
    SDL_UnlockAudioDevice(ctx->device->sdldevice);

    for (blocki = 0; blocki < ctx->num_source_blocks; blocki++) {
        SourceBlock *sb = ctx->source_blocks[blocki];
        if (sb->used > 0) {
            ALsizei i;
            for (i = 0; i < SDL_arraysize(sb->sources); i++) {
                ALsource *src = &sb->sources[i];
                if (!src->allocated) {
                    continue;
                }

                if (src->src_state)
                    src_delete(src->src_state);
                source_release_buffer_queue(ctx, src);
                if (--sb->used == 0) {
                    break;
                }
            }
        }
        free_simd_aligned(sb);
    }

    SDL_DestroyMutex(ctx->source_lock);
    SDL_free(ctx->source_blocks);
    SDL_free(ctx->attributes);
    free_simd_aligned(ctx);
}
ENTRYPOINTVOID(alcDestroyContext,(ALCcontext *ctx),(ctx))

/* no api lock; atomic. */
ALCcontext *alcGetCurrentContext(void)
{
    return get_current_context();
}

/* no api lock; immutable. */
ALCdevice *alcGetContextsDevice(ALCcontext *context)
{
    return context ? context->device : NULL;
}

static ALCenum _alcGetError(ALCdevice *device)
{
    ALCenum *perr = device ? &device->error : &null_device_error;
    const ALCenum retval = *perr;
    *perr = ALC_NO_ERROR;
    return retval;
}
ENTRYPOINT(ALCenum,alcGetError,(ALCdevice *device),(device))

/* no api lock; immutable */
ALCboolean alcIsExtensionPresent(ALCdevice *device, const ALCchar *extname)
{
    #define ALC_EXTENSION_ITEM(ext) if (SDL_strcasecmp(extname, #ext) == 0) { return ALC_TRUE; }
    ALC_EXTENSION_ITEMS
    #undef ALC_EXTENSION_ITEM
    return ALC_FALSE;
}

/* no api lock; immutable */
void *alcGetProcAddress(ALCdevice *device, const ALCchar *funcname)
{
    if (!funcname) {
        set_alc_error(device, ALC_INVALID_VALUE);
        return NULL;
    }

    #define FN_TEST(fn) if (SDL_strcmp(funcname, #fn) == 0) return (void *) fn
    FN_TEST(alcCreateContext);
    FN_TEST(alcMakeContextCurrent);
    FN_TEST(alcProcessContext);
    FN_TEST(alcSuspendContext);
    FN_TEST(alcDestroyContext);
    FN_TEST(alcGetCurrentContext);
    FN_TEST(alcGetContextsDevice);
    FN_TEST(alcOpenDevice);
    FN_TEST(alcCloseDevice);
    FN_TEST(alcGetError);
    FN_TEST(alcIsExtensionPresent);
    FN_TEST(alcGetProcAddress);
    FN_TEST(alcGetEnumValue);
    FN_TEST(alcGetString);
    FN_TEST(alcGetIntegerv);
    FN_TEST(alcCaptureOpenDevice);
    FN_TEST(alcCaptureCloseDevice);
    FN_TEST(alcCaptureStart);
    FN_TEST(alcCaptureStop);
    FN_TEST(alcCaptureSamples);
    #undef FN_TEST

    set_alc_error(device, ALC_INVALID_VALUE);
    return NULL;
}

/* no api lock; immutable */
ALCenum alcGetEnumValue(ALCdevice *device, const ALCchar *enumname)
{
    if (!enumname) {
        set_alc_error(device, ALC_INVALID_VALUE);
        return (ALCenum) AL_NONE;
    }

    #define ENUM_TEST(en) if (SDL_strcmp(enumname, #en) == 0) return en
    ENUM_TEST(ALC_FALSE);
    ENUM_TEST(ALC_TRUE);
    ENUM_TEST(ALC_FREQUENCY);
    ENUM_TEST(ALC_REFRESH);
    ENUM_TEST(ALC_SYNC);
    ENUM_TEST(ALC_MONO_SOURCES);
    ENUM_TEST(ALC_STEREO_SOURCES);
    ENUM_TEST(ALC_NO_ERROR);
    ENUM_TEST(ALC_INVALID_DEVICE);
    ENUM_TEST(ALC_INVALID_CONTEXT);
    ENUM_TEST(ALC_INVALID_ENUM);
    ENUM_TEST(ALC_INVALID_VALUE);
    ENUM_TEST(ALC_OUT_OF_MEMORY);
    ENUM_TEST(ALC_MAJOR_VERSION);
    ENUM_TEST(ALC_MINOR_VERSION);
    ENUM_TEST(ALC_ATTRIBUTES_SIZE);
    ENUM_TEST(ALC_ALL_ATTRIBUTES);
    ENUM_TEST(ALC_DEFAULT_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_EXTENSIONS);
    ENUM_TEST(ALC_CAPTURE_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
    ENUM_TEST(ALC_CAPTURE_SAMPLES);
    ENUM_TEST(ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
    ENUM_TEST(ALC_ALL_DEVICES_SPECIFIER);
    ENUM_TEST(ALC_CONNECTED);
    #undef ENUM_TEST

    set_alc_error(device, ALC_INVALID_VALUE);
    return (ALCenum) AL_NONE;
}

static const ALCchar *calculate_sdl_device_list(const int iscapture)
{
    /* alcGetString() has to return a const string that is not freed and might
       continue to live even if we update this list in a later query, so we
       just make a big static buffer and hope it's large enough and that other
       race conditions don't bite us. The enumeration extension shouldn't have
       reused entry points, or done this silly null-delimited string list.
       Oh well. */
    #define DEVICE_LIST_BUFFER_SIZE 512
    static ALCchar playback_list[DEVICE_LIST_BUFFER_SIZE];
    static ALCchar capture_list[DEVICE_LIST_BUFFER_SIZE];
    ALCchar *final_list = iscapture ? capture_list : playback_list;
    ALCchar *ptr = final_list;
    int numdevs;
    size_t avail = DEVICE_LIST_BUFFER_SIZE;
    size_t cpy;
    int i;

    /* default device is always available. */
    cpy = SDL_strlcpy(ptr, iscapture ? DEFAULT_CAPTURE_DEVICE : DEFAULT_PLAYBACK_DEVICE, avail);
    SDL_assert((cpy+1) < avail);
    ptr += cpy + 1;  /* skip past null char. */
    avail -= cpy + 1;

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) == -1) {
        return NULL;
    }

    if (!init_api_lock()) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    grab_api_lock();

    numdevs = SDL_GetNumAudioDevices(iscapture);

    for (i = 0; i < numdevs; i++) {
        const char *devname = SDL_GetAudioDeviceName(i, iscapture);
        const size_t devnamelen = SDL_strlen(devname);
        /* if we're out of space, we just have to drop devices we can't cram in the buffer. */
        if (avail > (devnamelen + 2)) {
            cpy = SDL_strlcpy(ptr, devname, avail);
            SDL_assert(cpy == devnamelen);
            SDL_assert((cpy+1) < avail);
            ptr += cpy + 1;  /* skip past null char. */
            avail -= cpy + 1;
        }
    }

    SDL_assert(avail >= 1);
    *ptr = '\0';

    ungrab_api_lock();

    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return final_list;

    #undef DEVICE_LIST_BUFFER_SIZE
}

/* no api lock; immutable (unless it isn't, then we manually lock). */
const ALCchar *alcGetString(ALCdevice *device, ALCenum param)
{
    switch (param) {
        case ALC_EXTENSIONS: {
            #define ALC_EXTENSION_ITEM(ext) " " #ext
            static ALCchar alc_extensions_string[] = ALC_EXTENSION_ITEMS;
            #undef ALC_EXTENSION_ITEM
            return alc_extensions_string + 1;  /* skip that first space char */
        }

        /* You open the default SDL device with a NULL device name, but that is how OpenAL
           reports an error here, so we give it a magic identifier here instead. */
        case ALC_DEFAULT_DEVICE_SPECIFIER:
            return DEFAULT_PLAYBACK_DEVICE;

        case ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER:
            return DEFAULT_CAPTURE_DEVICE;

        case ALC_DEVICE_SPECIFIER:
            FIXME("should return NULL if device->iscapture?");
            return device ? device->name : calculate_sdl_device_list(0);

        case ALC_CAPTURE_DEVICE_SPECIFIER:
            FIXME("should return NULL if !device->iscapture?");
            return device ? device->name : calculate_sdl_device_list(1);

        case ALC_NO_ERROR: return "ALC_NO_ERROR";
        case ALC_INVALID_DEVICE: return "ALC_INVALID_DEVICE";
        case ALC_INVALID_CONTEXT:return "ALC_INVALID_CONTEXT";
        case ALC_INVALID_ENUM: return "ALC_INVALID_ENUM";
        case ALC_INVALID_VALUE: return "ALC_INVALID_VALUE";
        case ALC_OUT_OF_MEMORY: return "ALC_OUT_OF_MEMORY";

        default: break;
    }

    FIXME("other enums that should report as strings?");
    set_alc_error(device, ALC_INVALID_ENUM);
    return NULL;
}

static void _alcGetIntegerv(ALCdevice *device, const ALCenum param, const ALCsizei size, ALCint *values)
{
    ALCcontext *ctx = NULL;

    if (!size || !values) {
        return;  /* "A NULL destination or a zero size parameter will cause ALC to ignore the query." */
    }

    switch (param) {
        case ALC_CAPTURE_SAMPLES:
            if (!device || !device->iscapture) {
                set_alc_error(device, ALC_INVALID_DEVICE);
                return;
            }

            FIXME("make ring buffer atomic?");
            SDL_LockAudioDevice(device->sdldevice);
            *values = (ALCint) (device->capture.ring.used / device->framesize);
            SDL_UnlockAudioDevice(device->sdldevice);
            return;

        case ALC_CONNECTED:
            if (device) {
                *values = SDL_AtomicGet(&device->connected) ? ALC_TRUE : ALC_FALSE;
            } else {
                *values = ALC_FALSE;
                set_alc_error(device, ALC_INVALID_DEVICE);
            }
            return;

        case ALC_ATTRIBUTES_SIZE:
        case ALC_ALL_ATTRIBUTES:
            if (!device || device->iscapture) {
                *values = 0;
                set_alc_error(device, ALC_INVALID_DEVICE);
                return;
            }

            ctx = get_current_context();

            FIXME("wants 'current context of specified device', but there isn't a current context per-device...");
            if ((!ctx) || (ctx->device != device)) {
                *values = 0;
                set_alc_error(device, ALC_INVALID_CONTEXT);
                return;
            }

            if (param == ALC_ALL_ATTRIBUTES) {
                if (size < ctx->attributes_count) {
                    *values = 0;
                    set_alc_error(device, ALC_INVALID_VALUE);
                    return;
                }
                SDL_memcpy(values, ctx->attributes, ctx->attributes_count * sizeof (ALCint));
            } else {
                *values = (ALCint) ctx->attributes_count;
            }
            return;

        case ALC_MAJOR_VERSION:
            *values = OPENAL_VERSION_MAJOR;
            return;

        case ALC_MINOR_VERSION:
            *values = OPENAL_VERSION_MINOR;
            return;

        default: break;
    }

    set_alc_error(device, ALC_INVALID_ENUM);
    *values = 0;
}
ENTRYPOINTVOID(alcGetIntegerv,(ALCdevice *device, ALCenum param, ALCsizei size, ALCint *values),(device,param,size,values))


/* audio callback for capture devices just needs to move data into our
   ringbuffer for later recovery by the app in alcCaptureSamples(). SDL
   should have handled resampling and conversion for us to the expected
   audio format. */
static void SDLCALL capture_device_callback(void *userdata, Uint8 *stream, int len)
{
    ALCdevice *device = (ALCdevice *) userdata;
    ALCboolean connected = ALC_FALSE;
    SDL_assert(device->iscapture);

    if (SDL_AtomicGet(&device->connected)) {
        if (SDL_GetAudioDeviceStatus(device->sdldevice) == SDL_AUDIO_STOPPED) {
            SDL_AtomicSet(&device->connected, ALC_FALSE);
        } else {
            connected = ALC_TRUE;
        }
    }

    if (connected) {
        ring_buffer_put(&device->capture.ring, stream, (ALCsizei) len);
    }
}

/* no api lock; this creates it and otherwise doesn't have any state that can race */
ALCdevice *alcCaptureOpenDevice(const ALCchar *devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize)
{
    SDL_AudioSpec desired;
    ALCsizei framesize = 0;
    const char *sdldevname = NULL;
    ALCdevice *device = NULL;
    ALCubyte *ringbuf = NULL;

    SDL_zero(desired);
    if (!alcfmt_to_sdlfmt(format, &desired.format, &desired.channels, &framesize)) {
        return NULL;
    }

    if (!devicename) {
        devicename = DEFAULT_CAPTURE_DEVICE;  /* so ALC_CAPTURE_DEVICE_SPECIFIER is meaningful */
    }

    desired.freq = frequency;
    desired.samples = 1024;  FIXME("is this a reasonable value?");
    desired.callback = capture_device_callback;

    if (SDL_strcmp(devicename, DEFAULT_CAPTURE_DEVICE) != 0) {
        sdldevname = devicename;  /* we want NULL for the best SDL default unless app is explicit. */
    }

    device = prep_alc_device(devicename, ALC_TRUE);
    if (!device) {
        return NULL;
    }

    device->frequency = frequency;
    device->framesize = framesize;
    device->capture.ring.size = framesize * buffersize;

    if (device->capture.ring.size >= buffersize) {
        ringbuf = (ALCubyte *) SDL_malloc(device->capture.ring.size);
    }

    if (!ringbuf) {
        SDL_free(device->name);
        SDL_free(device);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    device->capture.ring.buffer = ringbuf;

    desired.userdata = device;

    device->sdldevice = SDL_OpenAudioDevice(sdldevname, 1, &desired, NULL, 0);
    if (!device->sdldevice) {
        SDL_free(ringbuf);
        SDL_free(device->name);
        SDL_free(device);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return NULL;
    }

    return device;
}

/* no api lock; this requires you to not destroy a device that's still in use */
ALCboolean alcCaptureCloseDevice(ALCdevice *device)
{
    if (!device || !device->iscapture) {
        return ALC_FALSE;
    }

    if (device->sdldevice) {
        SDL_CloseAudioDevice(device->sdldevice);
    }

    SDL_free(device->capture.ring.buffer);
    SDL_free(device->name);
    SDL_free(device);
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    return ALC_TRUE;
}

static void _alcCaptureStart(ALCdevice *device)
{
    if (device && device->iscapture) {
        /* alcCaptureStart() drops any previously-buffered data. */
        FIXME("does this clear the ring buffer if the device is already started?");
        SDL_LockAudioDevice(device->sdldevice);
        device->capture.ring.read = 0;
        device->capture.ring.write = 0;
        device->capture.ring.used = 0;
        SDL_UnlockAudioDevice(device->sdldevice);
        SDL_PauseAudioDevice(device->sdldevice, 0);
    }
}
ENTRYPOINTVOID(alcCaptureStart,(ALCdevice *device),(device))

static void _alcCaptureStop(ALCdevice *device)
{
    if (device && device->iscapture) {
        SDL_PauseAudioDevice(device->sdldevice, 1);
    }
}
ENTRYPOINTVOID(alcCaptureStop,(ALCdevice *device),(device))

static void _alcCaptureSamples(ALCdevice *device, ALCvoid *buffer, const ALCsizei samples)
{
    ALCsizei requested_bytes;
    if (!device || !device->iscapture) {
        return;
    }

    requested_bytes = samples * device->framesize;

    SDL_LockAudioDevice(device->sdldevice);
    if (requested_bytes > device->capture.ring.used) {
        SDL_UnlockAudioDevice(device->sdldevice);
        FIXME("set error state?");
        return;  /* this is an error state, according to the spec. */
    }

    ring_buffer_get(&device->capture.ring, buffer, requested_bytes);
    SDL_UnlockAudioDevice(device->sdldevice);
}
ENTRYPOINTVOID(alcCaptureSamples,(ALCdevice *device, ALCvoid *buffer, ALCsizei samples),(device,buffer,samples))


/* AL implementation... */

static ALenum null_context_error = AL_NO_ERROR;

static void set_al_error(ALCcontext *ctx, const ALenum error)
{
    ALenum *perr = ctx ? &ctx->error : &null_context_error;
    /* can't set a new error when the previous hasn't been cleared yet. */
    if (*perr == AL_NO_ERROR) {
        *perr = error;
    }
}

/* !!! FIXME: buffers and sources use almost identical code for blocks */
static ALsource *get_source(ALCcontext *ctx, const ALuint name, SourceBlock **_block)
{
    const ALsizei blockidx = (((ALsizei) name) - 1) / OPENAL_SOURCE_BLOCK_SIZE;
    const ALsizei block_offset = (((ALsizei) name) - 1) % OPENAL_SOURCE_BLOCK_SIZE;
    ALsource *source;
    SourceBlock *block;

    /*printf("get_source(%d): blockidx=%d, block_offset=%d\n", (int) name, (int) blockidx, (int) block_offset);*/

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        if (_block) *_block = NULL;
        return NULL;
    } else if ((name == 0) || (blockidx >= ctx->num_source_blocks)) {
        set_al_error(ctx, AL_INVALID_NAME);
        if (_block) *_block = NULL;
        return NULL;
    }

    block = ctx->source_blocks[blockidx];
    source = &block->sources[block_offset];
    if (source->allocated) {
        if (_block) *_block = block;
        return source;
    }

    if (_block) *_block = NULL;
    set_al_error(ctx, AL_INVALID_NAME);
    return NULL;
}

/* !!! FIXME: buffers and sources use almost identical code for blocks */
static ALbuffer *get_buffer(ALCcontext *ctx, const ALuint name, BufferBlock **_block)
{
    const ALsizei blockidx = (((ALsizei) name) - 1) / OPENAL_BUFFER_BLOCK_SIZE;
    const ALsizei block_offset = (((ALsizei) name) - 1) % OPENAL_BUFFER_BLOCK_SIZE;
    ALbuffer *buffer;
    BufferBlock *block;

    /*printf("get_buffer(%d): blockidx=%d, block_offset=%d\n", (int) name, (int) blockidx, (int) block_offset);*/

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        if (_block) *_block = NULL;
        return NULL;
    } else if ((name == 0) || (blockidx >= ctx->device->playback.num_buffer_blocks)) {
        set_al_error(ctx, AL_INVALID_NAME);
        if (_block) *_block = NULL;
        return NULL;
    }

    block = ctx->device->playback.buffer_blocks[blockidx];
    buffer = &block->buffers[block_offset];
    if (buffer->allocated) {
        if (_block) *_block = block;
        return buffer;
    }

    if (_block) *_block = NULL;
    set_al_error(ctx, AL_INVALID_NAME);
    return NULL;
}

static void _alDopplerFactor(const ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (value < 0.0f) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ctx->doppler_factor = value;
        context_needs_recalc(ctx);
    }
}
ENTRYPOINTVOID(alDopplerFactor,(ALfloat value),(value))

static void _alDopplerVelocity(const ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (value < 0.0f) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ctx->doppler_velocity = value;
        context_needs_recalc(ctx);
    }
}
ENTRYPOINTVOID(alDopplerVelocity,(ALfloat value),(value))

static void _alSpeedOfSound(const ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (value < 0.0f) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ctx->speed_of_sound = value;
        context_needs_recalc(ctx);
    }
}
ENTRYPOINTVOID(alSpeedOfSound,(ALfloat value),(value))

static void _alDistanceModel(const ALenum model)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    switch (model) {
        case AL_NONE:
        case AL_INVERSE_DISTANCE:
        case AL_INVERSE_DISTANCE_CLAMPED:
        case AL_LINEAR_DISTANCE:
        case AL_LINEAR_DISTANCE_CLAMPED:
        case AL_EXPONENT_DISTANCE:
        case AL_EXPONENT_DISTANCE_CLAMPED:
            ctx->distance_model = model;
            context_needs_recalc(ctx);
            return;
        default: break;
    }
    set_al_error(ctx, AL_INVALID_ENUM);
}
ENTRYPOINTVOID(alDistanceModel,(ALenum model),(model))


static void _alEnable(const ALenum capability)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alEnable,(ALenum capability),(capability))


static void _alDisable(const ALenum capability)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alDisable,(ALenum capability),(capability))


static ALboolean _alIsEnabled(const ALenum capability)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
    return AL_FALSE;
}
ENTRYPOINT(ALboolean,alIsEnabled,(ALenum capability),(capability))

static const ALchar *_alGetString(const ALenum param)
{
    switch (param) {
        case AL_EXTENSIONS: {
            #define AL_EXTENSION_ITEM(ext) " " #ext
            static ALchar al_extensions_string[] = AL_EXTENSION_ITEMS;
            #undef AL_EXTENSION_ITEM
            return al_extensions_string + 1;  /* skip that first space char */
        }

        case AL_VERSION: return OPENAL_VERSION_STRING;
        case AL_RENDERER: return OPENAL_RENDERER_STRING;
        case AL_VENDOR: return OPENAL_VENDOR_STRING;
        case AL_NO_ERROR: return "AL_NO_ERROR";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";

        default: break;
    }

    FIXME("other enums that should report as strings?");
    set_al_error(get_current_context(), AL_INVALID_ENUM);

    return NULL;
}
ENTRYPOINT(const ALchar *,alGetString,(const ALenum param),(param))

static void _alGetBooleanv(const ALenum param, ALboolean *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  /* legal no-op */

    /* nothing in core OpenAL 1.1 uses this */
    set_al_error(ctx, AL_INVALID_ENUM);
}
ENTRYPOINTVOID(alGetBooleanv,(ALenum param, ALboolean *values),(param,values))

static void _alGetIntegerv(const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  /* legal no-op */

    switch (param) {
        case AL_DISTANCE_MODEL: *values = (ALint) ctx->distance_model; break;
        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetIntegerv,(ALenum param, ALint *values),(param,values))

static void _alGetFloatv(const ALenum param, ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  /* legal no-op */

    switch (param) {
        case AL_DOPPLER_FACTOR: *values = ctx->doppler_factor; break;
        case AL_DOPPLER_VELOCITY: *values = ctx->doppler_velocity; break;
        case AL_SPEED_OF_SOUND: *values = ctx->speed_of_sound; break;
        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetFloatv,(ALenum param, ALfloat *values),(param,values))

static void _alGetDoublev(const ALenum param, ALdouble *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  /* legal no-op */

    /* nothing in core OpenAL 1.1 uses this */
    set_al_error(ctx, AL_INVALID_ENUM);
}
ENTRYPOINTVOID(alGetDoublev,(ALenum param, ALdouble *values),(param,values))

/* no api lock; just passes through to the real api */
ALboolean alGetBoolean(ALenum param)
{
    ALboolean retval = AL_FALSE;
    alGetBooleanv(param, &retval);
    return retval;
}

/* no api lock; just passes through to the real api */
ALint alGetInteger(ALenum param)
{
    ALint retval = 0;
    alGetIntegerv(param, &retval);
    return retval;
}

/* no api lock; just passes through to the real api */
ALfloat alGetFloat(ALenum param)
{
    ALfloat retval = 0.0f;
    alGetFloatv(param, &retval);
    return retval;
}

/* no api lock; just passes through to the real api */
ALdouble alGetDouble(ALenum param)
{
    ALdouble retval = 0.0f;
    alGetDoublev(param, &retval);
    return retval;
}

static ALenum _alGetError(void)
{
    ALCcontext *ctx = get_current_context();
    ALenum *perr = ctx ? &ctx->error : &null_context_error;
    const ALenum retval = *perr;
    *perr = AL_NO_ERROR;
    return retval;
}
ENTRYPOINT(ALenum,alGetError,(void),())

/* no api lock; immutable (unless we start having contexts with different extensions) */
ALboolean alIsExtensionPresent(const ALchar *extname)
{
    #define AL_EXTENSION_ITEM(ext) if (SDL_strcasecmp(extname, #ext) == 0) { return AL_TRUE; }
    AL_EXTENSION_ITEMS
    #undef AL_EXTENSION_ITEM
    return AL_FALSE;
}

static void *_alGetProcAddress(const ALchar *funcname)
{
    ALCcontext *ctx = get_current_context();
    FIXME("fail if ctx == NULL?");
    if (!funcname) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return NULL;
    }

    #define FN_TEST(fn) if (SDL_strcmp(funcname, #fn) == 0) return (void *) fn
    FN_TEST(alDopplerFactor);
    FN_TEST(alDopplerVelocity);
    FN_TEST(alSpeedOfSound);
    FN_TEST(alDistanceModel);
    FN_TEST(alEnable);
    FN_TEST(alDisable);
    FN_TEST(alIsEnabled);
    FN_TEST(alGetString);
    FN_TEST(alGetBooleanv);
    FN_TEST(alGetIntegerv);
    FN_TEST(alGetFloatv);
    FN_TEST(alGetDoublev);
    FN_TEST(alGetBoolean);
    FN_TEST(alGetInteger);
    FN_TEST(alGetFloat);
    FN_TEST(alGetDouble);
    FN_TEST(alGetError);
    FN_TEST(alIsExtensionPresent);
    FN_TEST(alGetProcAddress);
    FN_TEST(alGetEnumValue);
    FN_TEST(alListenerf);
    FN_TEST(alListener3f);
    FN_TEST(alListenerfv);
    FN_TEST(alListeneri);
    FN_TEST(alListener3i);
    FN_TEST(alListeneriv);
    FN_TEST(alGetListenerf);
    FN_TEST(alGetListener3f);
    FN_TEST(alGetListenerfv);
    FN_TEST(alGetListeneri);
    FN_TEST(alGetListener3i);
    FN_TEST(alGetListeneriv);
    FN_TEST(alGenSources);
    FN_TEST(alDeleteSources);
    FN_TEST(alIsSource);
    FN_TEST(alSourcef);
    FN_TEST(alSource3f);
    FN_TEST(alSourcefv);
    FN_TEST(alSourcei);
    FN_TEST(alSource3i);
    FN_TEST(alSourceiv);
    FN_TEST(alGetSourcef);
    FN_TEST(alGetSource3f);
    FN_TEST(alGetSourcefv);
    FN_TEST(alGetSourcei);
    FN_TEST(alGetSource3i);
    FN_TEST(alGetSourceiv);
    FN_TEST(alSourcePlayv);
    FN_TEST(alSourceStopv);
    FN_TEST(alSourceRewindv);
    FN_TEST(alSourcePausev);
    FN_TEST(alSourcePlay);
    FN_TEST(alSourceStop);
    FN_TEST(alSourceRewind);
    FN_TEST(alSourcePause);
    FN_TEST(alSourceQueueBuffers);
    FN_TEST(alSourceUnqueueBuffers);
    FN_TEST(alGenBuffers);
    FN_TEST(alDeleteBuffers);
    FN_TEST(alIsBuffer);
    FN_TEST(alBufferData);
    FN_TEST(alBufferf);
    FN_TEST(alBuffer3f);
    FN_TEST(alBufferfv);
    FN_TEST(alBufferi);
    FN_TEST(alBuffer3i);
    FN_TEST(alBufferiv);
    FN_TEST(alGetBufferf);
    FN_TEST(alGetBuffer3f);
    FN_TEST(alGetBufferfv);
    FN_TEST(alGetBufferi);
    FN_TEST(alGetBuffer3i);
    FN_TEST(alGetBufferiv);
    #undef FN_TEST

    set_al_error(ctx, ALC_INVALID_VALUE);
    return NULL;
}
ENTRYPOINT(void *,alGetProcAddress,(const ALchar *funcname),(funcname))

static ALenum _alGetEnumValue(const ALchar *enumname)
{
    ALCcontext *ctx = get_current_context();
    FIXME("fail if ctx == NULL?");
    if (!enumname) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return AL_NONE;
    }

    #define ENUM_TEST(en) if (SDL_strcmp(enumname, #en) == 0) return en
    ENUM_TEST(AL_NONE);
    ENUM_TEST(AL_FALSE);
    ENUM_TEST(AL_TRUE);
    ENUM_TEST(AL_SOURCE_RELATIVE);
    ENUM_TEST(AL_CONE_INNER_ANGLE);
    ENUM_TEST(AL_CONE_OUTER_ANGLE);
    ENUM_TEST(AL_PITCH);
    ENUM_TEST(AL_POSITION);
    ENUM_TEST(AL_DIRECTION);
    ENUM_TEST(AL_VELOCITY);
    ENUM_TEST(AL_LOOPING);
    ENUM_TEST(AL_BUFFER);
    ENUM_TEST(AL_GAIN);
    ENUM_TEST(AL_MIN_GAIN);
    ENUM_TEST(AL_MAX_GAIN);
    ENUM_TEST(AL_ORIENTATION);
    ENUM_TEST(AL_SOURCE_STATE);
    ENUM_TEST(AL_INITIAL);
    ENUM_TEST(AL_PLAYING);
    ENUM_TEST(AL_PAUSED);
    ENUM_TEST(AL_STOPPED);
    ENUM_TEST(AL_BUFFERS_QUEUED);
    ENUM_TEST(AL_BUFFERS_PROCESSED);
    ENUM_TEST(AL_REFERENCE_DISTANCE);
    ENUM_TEST(AL_ROLLOFF_FACTOR);
    ENUM_TEST(AL_CONE_OUTER_GAIN);
    ENUM_TEST(AL_MAX_DISTANCE);
    ENUM_TEST(AL_SEC_OFFSET);
    ENUM_TEST(AL_SAMPLE_OFFSET);
    ENUM_TEST(AL_BYTE_OFFSET);
    ENUM_TEST(AL_SOURCE_TYPE);
    ENUM_TEST(AL_STATIC);
    ENUM_TEST(AL_STREAMING);
    ENUM_TEST(AL_UNDETERMINED);
    ENUM_TEST(AL_FORMAT_MONO8);
    ENUM_TEST(AL_FORMAT_MONO16);
    ENUM_TEST(AL_FORMAT_STEREO8);
    ENUM_TEST(AL_FORMAT_STEREO16);
    ENUM_TEST(AL_FREQUENCY);
    ENUM_TEST(AL_BITS);
    ENUM_TEST(AL_CHANNELS);
    ENUM_TEST(AL_SIZE);
    ENUM_TEST(AL_UNUSED);
    ENUM_TEST(AL_PENDING);
    ENUM_TEST(AL_PROCESSED);
    ENUM_TEST(AL_NO_ERROR);
    ENUM_TEST(AL_INVALID_NAME);
    ENUM_TEST(AL_INVALID_ENUM);
    ENUM_TEST(AL_INVALID_VALUE);
    ENUM_TEST(AL_INVALID_OPERATION);
    ENUM_TEST(AL_OUT_OF_MEMORY);
    ENUM_TEST(AL_VENDOR);
    ENUM_TEST(AL_VERSION);
    ENUM_TEST(AL_RENDERER);
    ENUM_TEST(AL_EXTENSIONS);
    ENUM_TEST(AL_DOPPLER_FACTOR);
    ENUM_TEST(AL_DOPPLER_VELOCITY);
    ENUM_TEST(AL_SPEED_OF_SOUND);
    ENUM_TEST(AL_DISTANCE_MODEL);
    ENUM_TEST(AL_INVERSE_DISTANCE);
    ENUM_TEST(AL_INVERSE_DISTANCE_CLAMPED);
    ENUM_TEST(AL_LINEAR_DISTANCE);
    ENUM_TEST(AL_LINEAR_DISTANCE_CLAMPED);
    ENUM_TEST(AL_EXPONENT_DISTANCE);
    ENUM_TEST(AL_EXPONENT_DISTANCE_CLAMPED);
    ENUM_TEST(AL_FORMAT_MONO_FLOAT32);
    ENUM_TEST(AL_FORMAT_STEREO_FLOAT32);
    #undef ENUM_TEST

    set_al_error(ctx, AL_INVALID_VALUE);
    return AL_NONE;
}
ENTRYPOINT(ALenum,alGetEnumValue,(const ALchar *enumname),(enumname))

static void _alListenerfv(const ALenum param, const ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (!values) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ALboolean recalc = AL_TRUE;
        switch (param) {
            case AL_GAIN:
                ctx->listener.gain = *values;
                break;

            case AL_POSITION:
                SDL_memcpy(ctx->listener.position, values, sizeof (*values) * 3);
                break;

            case AL_VELOCITY:
                SDL_memcpy(ctx->listener.velocity, values, sizeof (*values) * 3);
                break;

            case AL_ORIENTATION:
                SDL_memcpy(&ctx->listener.orientation[0], &values[0], sizeof (*values) * 3);
                SDL_memcpy(&ctx->listener.orientation[4], &values[3], sizeof (*values) * 3);
                break;

            default:
                recalc = AL_FALSE;
                set_al_error(ctx, AL_INVALID_ENUM);
                break;
        }

        if (recalc) {
            context_needs_recalc(ctx);
        }
    }
}
ENTRYPOINTVOID(alListenerfv,(ALenum param, const ALfloat *values),(param,values))

static void _alListenerf(const ALenum param, const ALfloat value)
{
    switch (param) {
        case AL_GAIN: _alListenerfv(param, &value); break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alListenerf,(ALenum param, ALfloat value),(param,value))

static void _alListener3f(const ALenum param, const ALfloat value1, const ALfloat value2, const ALfloat value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY: {
            const ALfloat values[3] = { value1, value2, value3 };
            _alListenerfv(param, values);
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alListener3f,(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3),(param,value1,value2,value3))

static void _alListeneriv(const ALenum param, const ALint *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
    } else if (!values) {
        set_al_error(ctx, AL_INVALID_VALUE);
    } else {
        ALboolean recalc = AL_TRUE;
        FIXME("Not atomic vs the mixer thread");  /* maybe have a latching system? */
        switch (param) {
            case AL_POSITION:
                ctx->listener.position[0] = (ALfloat) values[0];
                ctx->listener.position[1] = (ALfloat) values[1];
                ctx->listener.position[2] = (ALfloat) values[2];
                break;

            case AL_VELOCITY:
                ctx->listener.velocity[0] = (ALfloat) values[0];
                ctx->listener.velocity[1] = (ALfloat) values[1];
                ctx->listener.velocity[2] = (ALfloat) values[2];
                break;

            case AL_ORIENTATION:
                ctx->listener.orientation[0] = (ALfloat) values[0];
                ctx->listener.orientation[1] = (ALfloat) values[1];
                ctx->listener.orientation[2] = (ALfloat) values[2];
                ctx->listener.orientation[4] = (ALfloat) values[3];
                ctx->listener.orientation[5] = (ALfloat) values[4];
                ctx->listener.orientation[6] = (ALfloat) values[5];
                break;

            default:
                recalc = AL_FALSE;
                set_al_error(ctx, AL_INVALID_ENUM);
                break;
        }

        if (recalc) {
            context_needs_recalc(ctx);
        }
    }
}
ENTRYPOINTVOID(alListeneriv,(ALenum param, const ALint *values),(param,values))

static void _alListeneri(const ALenum param, const ALint value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in AL 1.1 uses this */
}
ENTRYPOINTVOID(alListeneri,(ALenum param, ALint value),(param,value))

static void _alListener3i(const ALenum param, const ALint value1, const ALint value2, const ALint value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY: {
            const ALint values[3] = { value1, value2, value3 };
            _alListeneriv(param, values);
            break;
        }
        default:
            set_al_error(get_current_context(), AL_INVALID_ENUM);
            break;
    }
}
ENTRYPOINTVOID(alListener3i,(ALenum param, ALint value1, ALint value2, ALint value3),(param,value1,value2,value3))

static void _alGetListenerfv(const ALenum param, ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  /* legal no-op */

    switch (param) {
        case AL_GAIN:
            *values = ctx->listener.gain;
            break;

        case AL_POSITION:
            SDL_memcpy(values, ctx->listener.position, sizeof (ALfloat) * 3);
            break;

        case AL_VELOCITY:
            SDL_memcpy(values, ctx->listener.velocity, sizeof (ALfloat) * 3);
            break;

        case AL_ORIENTATION:
            SDL_memcpy(&values[0], &ctx->listener.orientation[0], sizeof (ALfloat) * 3);
            SDL_memcpy(&values[3], &ctx->listener.orientation[4], sizeof (ALfloat) * 3);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListenerfv,(ALenum param, ALfloat *values),(param,values))

static void _alGetListenerf(const ALenum param, ALfloat *value)
{
    switch (param) {
        case AL_GAIN: _alGetListenerfv(param, value); break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListenerf,(ALenum param, ALfloat *value),(param,value))


static void _alGetListener3f(const ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    ALfloat values[3];
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
            _alGetListenerfv(param, values);
            if (value1) *value1 = values[0];
            if (value2) *value2 = values[1];
            if (value3) *value3 = values[2];
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListener3f,(ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3),(param,value1,value2,value3))


static void _alGetListeneri(const ALenum param, ALint *value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in AL 1.1 uses this */
}
ENTRYPOINTVOID(alGetListeneri,(ALenum param, ALint *value),(param,value))


static void _alGetListeneriv(const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (!values) return;  /* legal no-op */

    switch (param) {
        case AL_POSITION:
            values[0] = (ALint) ctx->listener.position[0];
            values[1] = (ALint) ctx->listener.position[1];
            values[2] = (ALint) ctx->listener.position[2];
            break;

        case AL_VELOCITY:
            values[0] = (ALint) ctx->listener.velocity[0];
            values[1] = (ALint) ctx->listener.velocity[1];
            values[2] = (ALint) ctx->listener.velocity[2];
            break;

        case AL_ORIENTATION:
            values[0] = (ALint) ctx->listener.orientation[0];
            values[1] = (ALint) ctx->listener.orientation[1];
            values[2] = (ALint) ctx->listener.orientation[2];
            values[3] = (ALint) ctx->listener.orientation[4];
            values[4] = (ALint) ctx->listener.orientation[5];
            values[5] = (ALint) ctx->listener.orientation[6];
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListeneriv,(ALenum param, ALint *values),(param,values))

static void _alGetListener3i(const ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    ALint values[3];
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
            _alGetListeneriv(param, values);
            if (value1) *value1 = values[0];
            if (value2) *value2 = values[1];
            if (value3) *value3 = values[2];
            break;

        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetListener3i,(ALenum param, ALint *value1, ALint *value2, ALint *value3),(param,value1,value2,value3))

/* !!! FIXME: buffers and sources use almost identical code for blocks */
static void _alGenSources(const ALsizei n, ALuint *names)
{
    ALCcontext *ctx = get_current_context();
    ALboolean out_of_memory = AL_FALSE;
    ALsizei totalblocks;
    ALsource *stackobjs[16];
    ALsource **objects = stackobjs;
    ALsizei found = 0;
    ALsizei block_offset = 0;
    ALsizei blocki;
    ALsizei i;

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (n <= SDL_arraysize(stackobjs)) {
        SDL_memset(stackobjs, '\0', sizeof (ALsource *) * n);
    } else {
        objects = (ALsource **) SDL_calloc(n, sizeof (ALsource *));
        if (!objects) {
            set_al_error(ctx, AL_OUT_OF_MEMORY);
            return;
        }
    }

    totalblocks = ctx->num_source_blocks;
    for (blocki = 0; blocki < totalblocks; blocki++) {
        SourceBlock *block = ctx->source_blocks[blocki];
        block->tmp = 0;
        if (block->used < SDL_arraysize(block->sources)) {  /* skip if full */
            for (i = 0; i < SDL_arraysize(block->sources); i++) {
                /* if a playing source was deleted, it will still be marked mixer_accessible
                    until the mixer thread shuffles it out. Until then, the source isn't
                    available for reuse. */
                if (!block->sources[i].allocated && !SDL_AtomicGet(&block->sources[i].mixer_accessible)) {
                    block->tmp++;
                    objects[found] = &block->sources[i];
                    names[found++] = (i + block_offset) + 1;  /* +1 so it isn't zero. */
                    if (found == n) {
                        break;
                    }
                }
            }

            if (found == n) {
                break;
            }
        }

        block_offset += SDL_arraysize(block->sources);
    }

    while (found < n) {  /* out of blocks? Add new ones. */
        /* ctx->source_blocks is only accessed on the API thread under a mutex, so it's safe to realloc. */
        void *ptr = SDL_realloc(ctx->source_blocks, sizeof (SourceBlock *) * (totalblocks + 1));
        SourceBlock *block;

        if (!ptr) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->source_blocks = (SourceBlock **) ptr;

        block = (SourceBlock *) calloc_simd_aligned(sizeof (SourceBlock));
        if (!block) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->source_blocks[totalblocks] = block;
        totalblocks++;
        ctx->num_source_blocks++;

        for (i = 0; i < SDL_arraysize(block->sources); i++) {
            block->tmp++;
            objects[found] = &block->sources[i];
            names[found++] = (i + block_offset) + 1;  /* +1 so it isn't zero. */
            if (found == n) {
                break;
            }
        }
        block_offset += SDL_arraysize(block->sources);
    }

    if (out_of_memory) {
        if (objects != stackobjs) SDL_free(objects);
        SDL_memset(names, '\0', sizeof (*names) * n);
        set_al_error(ctx, AL_OUT_OF_MEMORY);
        return;
    }

    SDL_assert(found == n);  /* we should have either gotten space or bailed on alloc failure */

    /* update the "used" field in blocks with items we are taking now. */
    found = 0;
    for (blocki = 0; found < n; blocki++) {
        SourceBlock *block = ctx->source_blocks[blocki];
        SDL_assert(blocki < totalblocks);
        const int foundhere = block->tmp;
        if (foundhere) {
            block->used += foundhere;
            found += foundhere;
            block->tmp = 0;
        }
    }

    SDL_assert(found == n);

    for (i = 0; i < n; i++) {
        ALsource *src = objects[i];

        /*printf("Generated source %u\n", (unsigned int) names[i]);*/

        SDL_assert(!src->allocated);

        /* Make sure everything that wants to use SIMD is aligned for it. */
        SDL_assert( (((size_t) &src->position[0]) % 16) == 0 );
        SDL_assert( (((size_t) &src->velocity[0]) % 16) == 0 );
        SDL_assert( (((size_t) &src->direction[0]) % 16) == 0 );

        SDL_zerop(src);
        SDL_AtomicSet(&src->state, AL_INITIAL);
        src->name = names[i];
        src->type = AL_UNDETERMINED;
        src->recalc = AL_TRUE;
        src->gain = 1.0f;
        src->max_gain = 1.0f;
        src->reference_distance = 1.0f;
        src->max_distance = FLT_MAX;
        src->rolloff_factor = 1.0f;
        src->pitch = 1.0f;
        src->cone_inner_angle = 360.0f;
        src->cone_outer_angle = 360.0f;
        source_needs_recalc(src);
        src->allocated = AL_TRUE;   /* we officially own it. */
    }

    if (objects != stackobjs) SDL_free(objects);
}
ENTRYPOINTVOID(alGenSources,(ALsizei n, ALuint *names),(n,names))


static void _alDeleteSources(const ALsizei n, const ALuint *names)
{
    ALCcontext *ctx = get_current_context();
    ALsizei i;

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    for (i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name == 0) {
            /* ignore it. */ FIXME("Spec says alDeleteBuffers() can have a zero name as a legal no-op, but this text isn't included in alDeleteSources...");
        } else {
            ALsource *source = get_source(ctx, name, NULL);
            if (!source) {
                /* "If one or more of the specified names is not valid, an AL_INVALID_NAME error will be recorded, and no objects will be deleted." */
                set_al_error(ctx, AL_INVALID_NAME);
                return;
            }
        }
    }

    for (i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name != 0) {
            SourceBlock *block;
            ALsource *source = get_source(ctx, name, &block);
            SDL_assert(source != NULL);

            /* "A playing source can be deleted--the source will be stopped automatically and then deleted." */
            if (!SDL_AtomicGet(&source->mixer_accessible)) {
                SDL_AtomicSet(&source->state, AL_STOPPED);
            } else {
                SDL_LockMutex(ctx->source_lock);
                SDL_AtomicSet(&source->state, AL_STOPPED);  /* mixer will drop from playlist next time it sees this. */
                SDL_UnlockMutex(ctx->source_lock);
            }
            source->allocated = AL_FALSE;
            source_release_buffer_queue(ctx, source);
            if (source->buffer) {
                SDL_assert(source->type == AL_STATIC);
                (void) SDL_AtomicDecRef(&source->buffer->refcount);
                source->buffer = NULL;
            }
            if (source->src_state) {
                src_delete(source->src_state);
                source->src_state = NULL;
            }
            block->used--;
        }
    }
}
ENTRYPOINTVOID(alDeleteSources,(ALsizei n, const ALuint *names),(n,names))

static ALboolean _alIsSource(const ALuint name)
{
    ALCcontext *ctx = get_current_context();
    return (ctx && (get_source(ctx, name, NULL) != NULL)) ? AL_TRUE : AL_FALSE;
}
ENTRYPOINT(ALboolean,alIsSource,(ALuint name),(name))

static void _alSourcefv(const ALuint name, const ALenum param, const ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) return;

    switch (param) {
        case AL_GAIN: src->gain = *values; break;
        case AL_POSITION: SDL_memcpy(src->position, values, sizeof (ALfloat) * 3); break;
        case AL_VELOCITY: SDL_memcpy(src->velocity, values, sizeof (ALfloat) * 3); break;
        case AL_DIRECTION: SDL_memcpy(src->direction, values, sizeof (ALfloat) * 3); break;
        case AL_MIN_GAIN: src->min_gain = *values; break;
        case AL_MAX_GAIN: src->max_gain = *values; break;
        case AL_REFERENCE_DISTANCE: src->reference_distance = *values; break;
        case AL_ROLLOFF_FACTOR: src->rolloff_factor = *values; break;
        case AL_MAX_DISTANCE: src->max_distance = *values; break;
        case AL_PITCH:
        {
            float pitch = *values;
            if (pitch < 0.5f)
                pitch = 0.5f;
            else if (pitch > 2.0f)
                pitch = 2.0f;
            src->pitch = pitch;
            break;
        }
        case AL_CONE_INNER_ANGLE: src->cone_inner_angle = *values; break;
        case AL_CONE_OUTER_ANGLE: src->cone_outer_angle = *values; break;
        case AL_CONE_OUTER_GAIN: src->cone_outer_gain = *values; break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            source_set_offset(src, param, *values);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); return;

    }

    source_needs_recalc(src);
}
ENTRYPOINTVOID(alSourcefv,(ALuint name, ALenum param, const ALfloat *values),(name,param,values))

static void _alSourcef(const ALuint name, const ALenum param, const ALfloat value)
{
    switch (param) {
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_PITCH:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alSourcefv(name, param, &value);
            break;

        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSourcef,(ALuint name, ALenum param, ALfloat value),(name,param,value))

static void _alSource3f(const ALuint name, const ALenum param, const ALfloat value1, const ALfloat value2, const ALfloat value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION: {
            const ALfloat values[3] = { value1, value2, value3 };
            _alSourcefv(name, param, values);
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSource3f,(ALuint name, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3),(name,param,value1,value2,value3))

static void set_source_static_buffer(ALCcontext *ctx, ALsource *src, const ALuint bufname)
{
    const ALenum state = (const ALenum) SDL_AtomicGet(&src->state);
    if ((state == AL_PLAYING) || (state == AL_PAUSED)) {
        set_al_error(ctx, AL_INVALID_OPERATION);  /* can't change buffer on playing/paused sources */
    } else {
        ALbuffer *buffer = NULL;
        if (bufname && ((buffer = get_buffer(ctx, bufname, NULL)) == NULL)) {
            set_al_error(ctx, AL_INVALID_VALUE);
        } else {
            const ALboolean must_lock = SDL_AtomicGet(&src->mixer_accessible) ? AL_TRUE : AL_FALSE;

            /* this can happen if you alSource(AL_BUFFER) while the exact source is in the middle of mixing */
            FIXME("Double-check this lock; we shouldn't be able to reach this if the source is playing.");
            if (must_lock) {
                SDL_LockMutex(ctx->source_lock);
            }

            if (src->buffer != buffer) {
                if (src->buffer) {
                    (void) SDL_AtomicDecRef(&src->buffer->refcount);
                }
                if (buffer) {
                    SDL_AtomicIncRef(&buffer->refcount);
                }
                src->buffer = buffer;
            }

            src->type = buffer ? AL_STATIC : AL_UNDETERMINED;
            src->queue_channels = buffer ? buffer->channels : 0;
            src->queue_frequency = 0;

            source_release_buffer_queue(ctx, src);

            if (src->src_state) {
                src_reset(src->src_state);
            }

            if (must_lock) {
                SDL_UnlockMutex(ctx->source_lock);
            }
        }
    }
}

static void _alSourceiv(const ALuint name, const ALenum param, const ALint *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) return;

    switch (param) {
        case AL_BUFFER: set_source_static_buffer(ctx, src, (ALuint) *values); break;
        case AL_SOURCE_RELATIVE: src->source_relative = *values ? AL_TRUE : AL_FALSE; break;
        case AL_LOOPING: src->looping = *values ? AL_TRUE : AL_FALSE; break;
        case AL_REFERENCE_DISTANCE: src->reference_distance = (ALfloat) *values; break;
        case AL_ROLLOFF_FACTOR: src->rolloff_factor = (ALfloat) *values; break;
        case AL_MAX_DISTANCE: src->max_distance = (ALfloat) *values; break;
        case AL_CONE_INNER_ANGLE: src->cone_inner_angle = (ALfloat) *values; break;
        case AL_CONE_OUTER_ANGLE: src->cone_outer_angle = (ALfloat) *values; break;

        case AL_DIRECTION:
            src->direction[0] = (ALfloat) values[0];
            src->direction[1] = (ALfloat) values[1];
            src->direction[2] = (ALfloat) values[2];
            break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            source_set_offset(src, param, (ALfloat)*values);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); return;
    }

    source_needs_recalc(src);
}
ENTRYPOINTVOID(alSourceiv,(ALuint name, ALenum param, const ALint *values),(name,param,values))

static void _alSourcei(const ALuint name, const ALenum param, const ALint value)
{
    switch (param) {
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_BUFFER:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alSourceiv(name, param, &value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSourcei,(ALuint name, ALenum param, ALint value),(name,param,value))

static void _alSource3i(const ALuint name, const ALenum param, const ALint value1, const ALint value2, const ALint value3)
{
    switch (param) {
        case AL_DIRECTION: {
            const ALint values[3] = { (ALint) value1, (ALint) value2, (ALint) value3 };
            _alSourceiv(name, param, values);
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alSource3i,(ALuint name, ALenum param, ALint value1, ALint value2, ALint value3),(name,param,value1,value2,value3))

static void _alGetSourcefv(const ALuint name, const ALenum param, ALfloat *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) return;

    switch (param) {
        case AL_GAIN: *values = src->gain; break;
        case AL_POSITION: SDL_memcpy(values, src->position, sizeof (ALfloat) * 3); break;
        case AL_VELOCITY: SDL_memcpy(values, src->velocity, sizeof (ALfloat) * 3); break;
        case AL_DIRECTION: SDL_memcpy(values, src->direction, sizeof (ALfloat) * 3); break;
        case AL_MIN_GAIN: *values = src->min_gain; break;
        case AL_MAX_GAIN: *values = src->max_gain; break;
        case AL_REFERENCE_DISTANCE: *values = src->reference_distance; break;
        case AL_ROLLOFF_FACTOR: *values = src->rolloff_factor; break;
        case AL_MAX_DISTANCE: *values = src->max_distance; break;
        case AL_PITCH: *values = src->pitch; break;
        case AL_CONE_INNER_ANGLE: *values = src->cone_inner_angle; break;
        case AL_CONE_OUTER_ANGLE: *values = src->cone_outer_angle; break;
        case AL_CONE_OUTER_GAIN:  *values = src->cone_outer_gain; break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            *values = source_get_offset(src, param);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourcefv,(ALuint name, ALenum param, ALfloat *values),(name,param,values))

static void _alGetSourcef(const ALuint name, const ALenum param, ALfloat *value)
{
    switch (param) {
        case AL_GAIN:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_PITCH:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_CONE_OUTER_GAIN:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alGetSourcefv(name, param, value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourcef,(ALuint name, ALenum param, ALfloat *value),(name,param,value))

static void _alGetSource3f(const ALuint name, const ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    switch (param) {
        case AL_POSITION:
        case AL_VELOCITY:
        case AL_DIRECTION: {
            ALfloat values[3];
            _alGetSourcefv(name, param, values);
            if (value1) *value1 = values[0];
            if (value2) *value2 = values[1];
            if (value3) *value3 = values[2];
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSource3f,(ALuint name, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3),(name,param,value1,value2,value3))

static void _alGetSourceiv(const ALuint name, const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) return;

    switch (param) {
        case AL_SOURCE_STATE: *values = (ALint) SDL_AtomicGet(&src->state); break;
        case AL_SOURCE_TYPE: *values = (ALint) src->type; break;
        case AL_BUFFER: *values = (ALint) (src->buffer ? src->buffer->name : 0); break;
        /* !!! FIXME: AL_BUFFERS_QUEUED is the total number of buffers pending, playing, and processed, so this is wrong. It might also have to be 1 if there's a static buffer, but I'm not sure. */
        case AL_BUFFERS_QUEUED: *values = (ALint) SDL_AtomicGet(&src->buffer_queue.num_items); break;
        case AL_BUFFERS_PROCESSED: *values = (ALint) SDL_AtomicGet(&src->buffer_queue_processed.num_items); break;
        case AL_SOURCE_RELATIVE: *values = (ALint) src->source_relative; break;
        case AL_LOOPING: *values = (ALint) src->looping; break;
        case AL_REFERENCE_DISTANCE: *values = (ALint) src->reference_distance; break;
        case AL_ROLLOFF_FACTOR: *values = (ALint) src->rolloff_factor; break;
        case AL_MAX_DISTANCE: *values = (ALint) src->max_distance; break;
        case AL_CONE_INNER_ANGLE: *values = (ALint) src->cone_inner_angle; break;
        case AL_CONE_OUTER_ANGLE: *values = (ALint) src->cone_outer_angle; break;
        case AL_DIRECTION:
            values[0] = (ALint) src->direction[0];
            values[1] = (ALint) src->direction[1];
            values[2] = (ALint) src->direction[2];
            break;

        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            *values = (ALint) source_get_offset(src, param);
            break;

        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourceiv,(ALuint name, ALenum param, ALint *values),(name,param,values))

static void _alGetSourcei(const ALuint name, const ALenum param, ALint *value)
{
    switch (param) {
        case AL_SOURCE_STATE:
        case AL_SOURCE_RELATIVE:
        case AL_LOOPING:
        case AL_BUFFER:
        case AL_BUFFERS_QUEUED:
        case AL_BUFFERS_PROCESSED:
        case AL_SOURCE_TYPE:
        case AL_REFERENCE_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_MAX_DISTANCE:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            _alGetSourceiv(name, param, value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSourcei,(ALuint name, ALenum param, ALint *value),(name,param,value))

static void _alGetSource3i(const ALuint name, const ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    switch (param) {
        case AL_DIRECTION: {
            ALint values[3];
            _alGetSourceiv(name, param, values);
            if (value1) *value1 = values[0];
            if (value2) *value2 = values[1];
            if (value3) *value3 = values[2];
            break;
        }
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetSource3i,(ALuint name, ALenum param, ALint *value1, ALint *value2, ALint *value3),(name,param,value1,value2,value3))

static void source_play(ALCcontext *ctx, const ALsizei n, const ALuint *names)
{
    ALboolean failed = AL_FALSE;
    SourcePlayTodo todo;
    SourcePlayTodo *todoend = &todo;
    SourcePlayTodo *todoptr;
    void *ptr;
    ALsizei i;

    if (n == 0) {
        return;
    } else if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    SDL_zero(todo);

    /* Obtain our SourcePlayTodo items upfront; if this runs out of
       memory, we won't have changed any state. The mixer thread will
       put items back in the pool when done with them, so this handoff needs
       to be atomic. */
    for (i = 0; i < n; i++) {
        SourcePlayTodo *item;
        do {
            ptr = SDL_AtomicGetPtr(&ctx->device->playback.source_todo_pool);
            item = (SourcePlayTodo *) ptr;
            if (!item) break;
            ptr = item->next;
        } while (!SDL_AtomicCASPtr(&ctx->device->playback.source_todo_pool, item, ptr));

        if (!item) {  /* allocate a new item */
            item = (SourcePlayTodo *) SDL_calloc(1, sizeof (SourcePlayTodo));
            if (!item) {
                set_al_error(ctx, AL_OUT_OF_MEMORY);
                failed = AL_TRUE;
                break;
            }
        }

        item->next = NULL;
        todoend->next = item;
        todoend = item;
    }

    if (failed) {
        /* put the whole new queue back in the pool for reuse later. */
        if (todo.next) {
            do {
                ptr = SDL_AtomicGetPtr(&ctx->device->playback.source_todo_pool);
                todoend->next = (SourcePlayTodo *) ptr;
            } while (!SDL_AtomicCASPtr(&ctx->device->playback.source_todo_pool, ptr, todo.next));
        }
        return;
    }

    FIXME("What do we do if there's an invalid source in the middle of the names vector?");
    for (i = 0, todoptr = todo.next; i < n; i++) {
        const ALuint name = names[i];
        ALsource *src = get_source(ctx, name, NULL);
        if (src) {
            if (src->offset_latched) {
                src->offset_latched = AL_FALSE;
            } else if (SDL_AtomicGet(&src->state) != AL_PAUSED) {
                src->offset = 0;
            }

            /* this used to move right to AL_STOPPED if the device is
               disconnected, but now we let the mixer thread handle that to
               avoid race conditions with marking the buffer queue
               processed, etc. Strictly speaking, ALC_EXT_disconnect
               says playing a source on a disconnected device should
               "immediately" progress to STOPPED, but I'm willing to
               say that the mixer will "immediately" move it as opposed to
               it stopping when the source would be done mixing (or worse:
               hang there forever). */
            SDL_AtomicSet(&src->state, AL_PLAYING);

            /* Mark this as visible to the mixer. This will be set back to zero by the mixer thread when it is done with the source. */
            SDL_AtomicSet(&src->mixer_accessible, 1);

            todoptr->source = src;
            todoptr = todoptr->next;
        }
    }

    /* Send the list to the mixer atomically, so all sources start playing in sync!
       We're going to put these on a linked list called playlist_todo
       The mixer does an atomiccasptr to grab the current list, swapping
       in a NULL. Once it has the list, it's safe to do what it likes
       with it, as nothing else owns the pointers in that list. */
    do {
        ptr = SDL_AtomicGetPtr(&ctx->playlist_todo);
        todoend->next = ptr;
    } while (!SDL_AtomicCASPtr(&ctx->playlist_todo, ptr, todo.next));
}

static void _alSourcePlay(const ALuint name)
{
    source_play(get_current_context(), 1, &name);
}
ENTRYPOINTVOID(alSourcePlay,(ALuint name),(name))

static void _alSourcePlayv(ALsizei n, const ALuint *names)
{
    source_play(get_current_context(), n, names);
}
ENTRYPOINTVOID(alSourcePlayv,(ALsizei n, const ALuint *names),(n, names))


static void source_stop(ALCcontext *ctx, const ALuint name)
{
    ALsource *src = get_source(ctx, name, NULL);
    if (src) {
        if (SDL_AtomicGet(&src->state) != AL_INITIAL) {
            const ALboolean must_lock = SDL_AtomicGet(&src->mixer_accessible) ? AL_TRUE : AL_FALSE;
            if (must_lock) {
                SDL_LockMutex(ctx->source_lock);
            }
            SDL_AtomicSet(&src->state, AL_STOPPED);
            source_mark_all_buffers_processed(src);
            if (must_lock) {
                SDL_UnlockMutex(ctx->source_lock);
            }
        }
    }
}

static void source_rewind(ALCcontext *ctx, const ALuint name)
{
    ALsource *src = get_source(ctx, name, NULL);
    if (src) {
        const ALboolean must_lock = SDL_AtomicGet(&src->mixer_accessible) ? AL_TRUE : AL_FALSE;
        if (must_lock) {
            SDL_LockMutex(ctx->source_lock);
        }
        SDL_AtomicSet(&src->state, AL_INITIAL);
        src->offset = 0;
        if (must_lock) {
            SDL_UnlockMutex(ctx->source_lock);
        }
    }
}

static void source_pause(ALCcontext *ctx, const ALuint name)
{
    ALsource *src = get_source(ctx, name, NULL);
    if (src) {
        SDL_AtomicCAS(&src->state, AL_PLAYING, AL_PAUSED);
    }
}

static float source_get_offset(ALsource *src, ALenum param)
{
    int offset = 0;
    int framesize = sizeof (float);
    int freq = 1;
    if (src->type == AL_STREAMING) {
        /* streaming: the offset counts from the first processed buffer in the queue. */
        BufferQueueItem *item = src->buffer_queue.head;
        if (item) {
            framesize = (int) (item->buffer->channels * sizeof (float));
            freq = (int) (item->buffer->frequency);
            int proc_buf = SDL_AtomicGet(&src->buffer_queue_processed.num_items);
            offset = (proc_buf * item->buffer->len + src->offset);
        }
    } else {
        framesize = (int) (src->buffer->channels * sizeof (float));
        freq = (int) src->buffer->frequency;
        offset = src->offset;
    }
    switch(param) {
        case AL_SAMPLE_OFFSET: return (float) (offset / framesize); break;
        case AL_SEC_OFFSET: return ((float) (offset / framesize)) / ((float) freq); break;
        case AL_BYTE_OFFSET: return (float) offset; break;
        default: break;
    }

    return 0.0f;
}

static void source_set_offset(ALsource *src, ALenum param, ALfloat value)
{
    ALCcontext *ctx = get_current_context();
    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (src->type == AL_STREAMING) {
        FIXME("set_offset for streaming sources not implemented");
        return;
    }

    const int bufflen = (int) src->buffer->len;
    const int framesize = (int) (src->buffer->channels * sizeof (float));
    const int freq = (int) src->buffer->frequency;
    int offset = -1;

    switch (param) {
        case AL_SAMPLE_OFFSET:
            offset = value * framesize;
            break;
        case AL_SEC_OFFSET:
            offset = value * freq * framesize;
            break;
        case AL_BYTE_OFFSET:
            offset = ((int)value / framesize) * framesize;
            break;
    }

    if ((offset < 0) || (offset > bufflen)) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    }

    /* make sure the offset lands on a sample frame boundary. */
    offset -= offset % framesize;

    if (!SDL_AtomicGet(&src->mixer_accessible)) {
        src->offset = offset;
    } else {
        SDL_LockMutex(ctx->source_lock);
        src->offset = offset;
        SDL_UnlockMutex(ctx->source_lock);
    }
}

/* deal with alSourcePlay and alSourcePlayv (etc) boiler plate... */
#define SOURCE_STATE_TRANSITION_OP(alfn, fn) \
    void alSource##alfn(ALuint name) { source_##fn(get_current_context(), name); } \
    void alSource##alfn##v(ALsizei n, const ALuint *sources) { \
        ALCcontext *ctx = get_current_context(); \
        if (!ctx) { \
            set_al_error(ctx, AL_INVALID_OPERATION); \
        } else { \
            ALsizei i; \
            if (n > 1) { \
                FIXME("Can we do this without a full device lock?"); \
                SDL_LockAudioDevice(ctx->device->sdldevice);  /* lock the SDL device so these all start mixing in the same callback. */ \
                for (i = 0; i < n; i++) { \
                    source_##fn(ctx, sources[i]); \
                } \
                SDL_UnlockAudioDevice(ctx->device->sdldevice); \
            } else if (n == 1) { \
                source_##fn(ctx, *sources); \
            } \
        } \
    }

SOURCE_STATE_TRANSITION_OP(Stop, stop)
SOURCE_STATE_TRANSITION_OP(Rewind, rewind)
SOURCE_STATE_TRANSITION_OP(Pause, pause)


static void _alSourceQueueBuffers(const ALuint name, const ALsizei nb, const ALuint *bufnames)
{
    BufferQueueItem *queue = NULL;
    BufferQueueItem *queueend = NULL;
    void *ptr;
    ALsizei i;
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    ALint queue_channels = 0;
    ALsizei queue_frequency = 0;
    ALboolean failed = AL_FALSE;

    if (!src) {
        return;
    }

    if (src->type == AL_STATIC) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (nb == 0) {
        return;  /* nothing to do. */
    }

    for (i = nb; i > 0; i--) {  /* build list in reverse */
        BufferQueueItem *item = NULL;
        const ALuint bufname = bufnames[i-1];
        ALbuffer *buffer = bufname ? get_buffer(ctx, bufname, NULL) : NULL;
        if (!buffer && bufname) {  /* uhoh, bad buffer name! */
            set_al_error(ctx, AL_INVALID_VALUE);
            failed = AL_TRUE;
            break;
        }

        if (buffer) {
            if (queue_channels == 0) {
                SDL_assert(queue_frequency == 0);
                queue_channels = buffer->channels;
                queue_frequency = buffer->frequency;
            } else if ((queue_channels != buffer->channels) || (queue_frequency != buffer->frequency)) {
                /* the whole queue must be the same format. */
                set_al_error(ctx, AL_INVALID_VALUE);
                failed = AL_TRUE;
                break;
            }
        }

        item = ctx->device->playback.buffer_queue_pool;
        if (item) {
            ctx->device->playback.buffer_queue_pool = item->next;
        } else {  /* allocate a new item */
            item = (BufferQueueItem *) SDL_calloc(1, sizeof (BufferQueueItem));
            if (!item) {
                set_al_error(ctx, AL_OUT_OF_MEMORY);
                failed = AL_TRUE;
                break;
            }
        }

        if (buffer) {
            SDL_AtomicIncRef(&buffer->refcount);  /* mark it as in-use. */
        }
        item->buffer = buffer;

        SDL_assert((queue != NULL) == (queueend != NULL));
        if (queueend) {
            queueend->next = item;
        } else {
            queue = item;
        }
        queueend = item;
    }

    if (!failed) {
        if (src->queue_frequency && queue_frequency) {  /* could be zero if we only queued AL name 0. */
            SDL_assert(src->queue_channels);
            SDL_assert(queue_channels);
            if ((src->queue_channels != queue_channels) || (src->queue_frequency != queue_frequency)) {
                set_al_error(ctx, AL_INVALID_VALUE);
                failed = AL_TRUE;
            }
        }
    }

    if (!src->queue_frequency) {
        SDL_assert(!src->queue_channels);
    }

    if (failed) {
        if (queue) {
            /* Drop our claim on any buffers we planned to queue. */
            BufferQueueItem *item;
            for (item = queue; item != NULL; item = item->next) {
                if (item->buffer) {
                    (void) SDL_AtomicDecRef(&item->buffer->refcount);
                }
            }

            /* put the whole new queue back in the pool for reuse later. */
            queueend->next = ctx->device->playback.buffer_queue_pool;
            ctx->device->playback.buffer_queue_pool = queue;
        }
        return;
    }

    FIXME("this needs to be set way sooner");

    FIXME("this used to have a source lock, think this one through");
    src->type = AL_STREAMING;

    if (!src->queue_channels) {
        src->queue_channels = queue_channels;
        src->queue_frequency = queue_frequency;
    }

    /* so we're going to put these on a linked list called just_queued,
        where things build up in reverse order, to keep this on a single
        pointer. The theory is we'll atomicgetptr the pointer, set that
        pointer as the "next" for our list, and then atomiccasptr our new
        list against the original pointer. If the CAS succeeds, we have
        a complete list, atomically set. If it fails, try again with
        the new pointer we found, updating our next pointer again. If it
        failed, it's because the pointer became NULL when the mixer thread
        grabbed the existing list.

        The mixer does an atomiccasptr to grab the current list, swapping
        in a NULL. Once it has the list, it's safe to do what it likes
        with it, as nothing else owns the pointers in that list. */

    do {
        ptr = SDL_AtomicGetPtr(&src->buffer_queue.just_queued);
        SDL_AtomicSetPtr(&queueend->next, ptr);
    } while (!SDL_AtomicCASPtr(&src->buffer_queue.just_queued, ptr, queue));

    SDL_AtomicAdd(&src->buffer_queue.num_items, (int) nb);
}
ENTRYPOINTVOID(alSourceQueueBuffers,(ALuint name, ALsizei nb, const ALuint *bufnames),(name,nb,bufnames))

static void _alSourceUnqueueBuffers(const ALuint name, const ALsizei nb, ALuint *bufnames)
{
    BufferQueueItem *queueend = NULL;
    BufferQueueItem *queue;
    BufferQueueItem *item;
    ALsizei i;
    ALCcontext *ctx = get_current_context();
    ALsource *src = get_source(ctx, name, NULL);
    if (!src) {
        return;
    }

    if (src->type == AL_STATIC) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (nb == 0) {
        return;  /* nothing to do. */
    }

    if (((ALsizei) SDL_AtomicGet(&src->buffer_queue_processed.num_items)) < nb) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    }

    SDL_AtomicAdd(&src->buffer_queue_processed.num_items, -((int) nb));

    obtain_newly_queued_buffers(&src->buffer_queue_processed);

    item = queue = src->buffer_queue_processed.head;
    for (i = 0; i < nb; i++) {
        /* buffer_queue_processed.num_items said list was long enough. */
        SDL_assert(item != NULL);
        item = item->next;
    }
    src->buffer_queue_processed.head = item;
    if (!item) {
        src->buffer_queue_processed.tail = NULL;
    }

    item = queue;
    for (i = 0; i < nb; i++) {
        if (item->buffer) {
            (void) SDL_AtomicDecRef(&item->buffer->refcount);
        }
        bufnames[i] = item->buffer ? item->buffer->name : 0;
        queueend = item;
        item = item->next;
    }

    /* put the whole new queue back in the pool for reuse later. */
    SDL_assert(queueend != NULL);
    queueend->next = ctx->device->playback.buffer_queue_pool;
    ctx->device->playback.buffer_queue_pool = queue;
}
ENTRYPOINTVOID(alSourceUnqueueBuffers,(ALuint name, ALsizei nb, ALuint *bufnames),(name,nb,bufnames))

/* !!! FIXME: buffers and sources use almost identical code for blocks */
static void _alGenBuffers(const ALsizei n, ALuint *names)
{
    ALCcontext *ctx = get_current_context();
    ALboolean out_of_memory = AL_FALSE;
    ALsizei totalblocks;
    ALbuffer *stackobjs[16];
    ALbuffer **objects = stackobjs;
    ALsizei found = 0;
    ALsizei block_offset = 0;
    ALsizei blocki;
    ALsizei i;

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    if (n <= SDL_arraysize(stackobjs)) {
        SDL_memset(stackobjs, '\0', sizeof (ALbuffer *) * n);
    } else {
        objects = (ALbuffer **) SDL_calloc(n, sizeof (ALbuffer *));
        if (!objects) {
            set_al_error(ctx, AL_OUT_OF_MEMORY);
            return;
        }
    }

    totalblocks = ctx->device->playback.num_buffer_blocks;
    for (blocki = 0; blocki < totalblocks; blocki++) {
        BufferBlock *block = ctx->device->playback.buffer_blocks[blocki];
        block->tmp = 0;
        if (block->used < SDL_arraysize(block->buffers)) {  /* skip if full */
            for (i = 0; i < SDL_arraysize(block->buffers); i++) {
                if (!block->buffers[i].allocated) {
                    block->tmp++;
                    objects[found] = &block->buffers[i];
                    names[found++] = (i + block_offset) + 1;  /* +1 so it isn't zero. */
                    if (found == n) {
                        break;
                    }
                }
            }

            if (found == n) {
                break;
            }
        }

        block_offset += SDL_arraysize(block->buffers);
    }

    while (found < n) {  /* out of blocks? Add new ones. */
        /* ctx->buffer_blocks is only accessed on the API thread under a mutex, so it's safe to realloc. */
        void *ptr = SDL_realloc(ctx->device->playback.buffer_blocks, sizeof (BufferBlock *) * (totalblocks + 1));
        BufferBlock *block;

        if (!ptr) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->device->playback.buffer_blocks = (BufferBlock **) ptr;

        block = (BufferBlock *) SDL_calloc(1, sizeof (BufferBlock));
        if (!block) {
            out_of_memory = AL_TRUE;
            break;
        }
        ctx->device->playback.buffer_blocks[totalblocks] = block;
        totalblocks++;
        ctx->device->playback.num_buffer_blocks++;

        for (i = 0; i < SDL_arraysize(block->buffers); i++) {
            block->tmp++;
            objects[found] = &block->buffers[i];
            names[found++] = (i + block_offset) + 1;  /* +1 so it isn't zero. */
            if (found == n) {
                break;
            }
        }
        block_offset += SDL_arraysize(block->buffers);
    }

    if (out_of_memory) {
        if (objects != stackobjs) SDL_free(objects);
        SDL_memset(names, '\0', sizeof (*names) * n);
        set_al_error(ctx, AL_OUT_OF_MEMORY);
        return;
    }

    SDL_assert(found == n);  /* we should have either gotten space or bailed on alloc failure */

    /* update the "used" field in blocks with items we are taking now. */
    found = 0;
    for (blocki = 0; found < n; blocki++) {
        BufferBlock *block = ctx->device->playback.buffer_blocks[blocki];
        SDL_assert(blocki < totalblocks);
        const int foundhere = block->tmp;
        if (foundhere) {
            block->used += foundhere;
            found += foundhere;
            block->tmp = 0;
        }
    }

    SDL_assert(found == n);

    for (i = 0; i < n; i++) {
        ALbuffer *buffer = objects[i];
        /*printf("Generated buffer %u\n", (unsigned int) names[i]);*/
        SDL_assert(!buffer->allocated);
        SDL_zerop(buffer);
        buffer->name = names[i];
        buffer->channels = 1;
        buffer->bits = 16;
        buffer->allocated = AL_TRUE;  /* we officially own it. */
    }

    if (objects != stackobjs) SDL_free(objects);
}
ENTRYPOINTVOID(alGenBuffers,(ALsizei n, ALuint *names),(n,names))

static void _alDeleteBuffers(const ALsizei n, const ALuint *names)
{
    ALCcontext *ctx = get_current_context();
    ALsizei i;

    if (!ctx) {
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    for (i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name == 0) {
            /* ignore it. */
        } else {
            ALbuffer *buffer = get_buffer(ctx, name, NULL);
            if (!buffer) {
                /* "If one or more of the specified names is not valid, an AL_INVALID_NAME error will be recorded, and no objects will be deleted." */
                set_al_error(ctx, AL_INVALID_NAME);
                return;
            } else if (SDL_AtomicGet(&buffer->refcount) != 0) {
                set_al_error(ctx, AL_INVALID_OPERATION);  /* still in use */
                return;
            }
        }
    }

    for (i = 0; i < n; i++) {
        const ALuint name = names[i];
        if (name != 0) {
            BufferBlock *block;
            ALbuffer *buffer = get_buffer(ctx, name, &block);
            void *data;
            SDL_assert(buffer != NULL);
            data = (void *) buffer->data;
            buffer->allocated = AL_FALSE;
            buffer->data = NULL;
            free_simd_aligned(data);
            block->used--;
        }
    }
}
ENTRYPOINTVOID(alDeleteBuffers,(ALsizei n, const ALuint *names),(n,names))

static ALboolean _alIsBuffer(ALuint name)
{
    ALCcontext *ctx = get_current_context();
    return (ctx && (get_buffer(ctx, name, NULL) != NULL)) ? AL_TRUE : AL_FALSE;
}
ENTRYPOINT(ALboolean,alIsBuffer,(ALuint name),(name))

static void _alBufferData(const ALuint name, const ALenum alfmt, const ALvoid *data, const ALsizei size, const ALsizei freq)
{
    ALCcontext *ctx = get_current_context();
    ALbuffer *buffer = get_buffer(ctx, name, NULL);
    SDL_AudioCVT sdlcvt;
    Uint8 channels;
    SDL_AudioFormat sdlfmt;
    ALCsizei framesize;
    int rc;
    int prevrefcount;

    if (!buffer) return;

    if (!alcfmt_to_sdlfmt(alfmt, &sdlfmt, &channels, &framesize)) {
        set_al_error(ctx, AL_INVALID_VALUE);
        return;
    }

    /* increment refcount so this can't be deleted or alBufferData'd from another thread */
    prevrefcount = SDL_AtomicIncRef(&buffer->refcount);
    SDL_assert(prevrefcount >= 0);
    if (prevrefcount != 0) {
        /* this buffer is being used by some source. Unqueue it first. */
        (void) SDL_AtomicDecRef(&buffer->refcount);
        set_al_error(ctx, AL_INVALID_OPERATION);
        return;
    }

    /* This check was from the wild west of lock-free programming, now we shouldn't pass get_buffer() if not allocated. */
    SDL_assert(buffer->allocated);

    /* right now we take a moment to convert the data to float32, since that's
       the format we want to work in, but we don't resample or change the channels */
    SDL_zero(sdlcvt);
    rc = SDL_BuildAudioCVT(&sdlcvt, sdlfmt, channels, (int) freq, AUDIO_F32SYS, channels, (int) freq);
    if (rc == -1) {
        (void) SDL_AtomicDecRef(&buffer->refcount);
        set_al_error(ctx, AL_OUT_OF_MEMORY);  /* not really, but oh well. */
        return;
    }

    sdlcvt.len = sdlcvt.len_cvt = size;
    sdlcvt.buf = (Uint8 *) calloc_simd_aligned(size * sdlcvt.len_mult);
    if (!sdlcvt.buf) {
        (void) SDL_AtomicDecRef(&buffer->refcount);
        set_al_error(ctx, AL_OUT_OF_MEMORY);
        return;
    }
    SDL_memcpy(sdlcvt.buf, data, size);

    if (rc == 1) {  /* conversion necessary */
        rc = SDL_ConvertAudio(&sdlcvt);
        SDL_assert(rc == 0);  /* this shouldn't fail. */
        if (sdlcvt.len_cvt < (size * sdlcvt.len_mult)) {  /* maybe shrink buffer */
            void *ptr = SDL_realloc(sdlcvt.buf, sdlcvt.len_cvt);
            if (ptr) {
                sdlcvt.buf = (Uint8 *) ptr;
            }
        }
    }

    free_simd_aligned((void *) buffer->data);  /* nuke any previous data. */
    buffer->data = (const float *) sdlcvt.buf;
    buffer->channels = (ALint) channels;
    buffer->bits = (ALint) SDL_AUDIO_BITSIZE(sdlfmt);  /* we're in float32, though. */
    buffer->frequency = freq;
    buffer->len = (ALsizei) sdlcvt.len_cvt;
    (void) SDL_AtomicDecRef(&buffer->refcount);  /* ready to go! */
}
ENTRYPOINTVOID(alBufferData,(ALuint name, ALenum alfmt, const ALvoid *data, ALsizei size, ALsizei freq),(name,alfmt,data,size,freq))

static void _alBufferfv(const ALuint name, const ALenum param, const ALfloat *values)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alBufferfv,(ALuint name, ALenum param, const ALfloat *values),(name,param,values))

static void _alBufferf(const ALuint name, const ALenum param, const ALfloat value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alBufferf,(ALuint name, ALenum param, ALfloat value),(name,param,value))

static void _alBuffer3f(const ALuint name, const ALenum param, const ALfloat value1, const ALfloat value2, const ALfloat value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alBuffer3f,(ALuint name, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3),(name,param,value1,value2,value3))

static void _alBufferiv(const ALuint name, const ALenum param, const ALint *values)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alBufferiv,(ALuint name, ALenum param, const ALint *values),(name,param,values))

static void _alBufferi(const ALuint name, const ALenum param, const ALint value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alBufferi,(ALuint name, ALenum param, ALint value),(name,param,value))

static void _alBuffer3i(const ALuint name, const ALenum param, const ALint value1, const ALint value2, const ALint value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alBuffer3i,(ALuint name, ALenum param, ALint value1, ALint value2, ALint value3),(name,param,value1,value2,value3))

static void _alGetBufferfv(const ALuint name, const ALenum param, const ALfloat *values)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alGetBufferfv,(ALuint name, ALenum param, ALfloat *values),(name,param,values))

static void _alGetBufferf(const ALuint name, const ALenum param, ALfloat *value)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alGetBufferf,(ALuint name, ALenum param, ALfloat *value),(name,param,value))

static void _alGetBuffer3f(const ALuint name, const ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM);  /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alGetBuffer3f,(ALuint name, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3),(name,param,value1,value2,value3))

static void _alGetBufferi(const ALuint name, const ALenum param, ALint *value)
{
    switch (param) {
        case AL_FREQUENCY:
        case AL_SIZE:
        case AL_BITS:
        case AL_CHANNELS:
            alGetBufferiv(name, param, value);
            break;
        default: set_al_error(get_current_context(), AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetBufferi,(ALuint name, ALenum param, ALint *value),(name,param,value))

static void _alGetBuffer3i(const ALuint name, const ALenum param, ALint *value1, ALint *value2, ALint *value3)
{
    set_al_error(get_current_context(), AL_INVALID_ENUM); /* nothing in core OpenAL 1.1 uses this */
}
ENTRYPOINTVOID(alGetBuffer3i,(ALuint name, ALenum param, ALint *value1, ALint *value2, ALint *value3),(name,param,value1,value2,value3))

static void _alGetBufferiv(const ALuint name, const ALenum param, ALint *values)
{
    ALCcontext *ctx = get_current_context();
    ALbuffer *buffer = get_buffer(ctx, name, NULL);
    if (!buffer) return;

    switch (param) {
        case AL_FREQUENCY: *values = (ALint) buffer->frequency; break;
        case AL_SIZE: *values = (ALint) buffer->len; break;
        case AL_BITS: *values = (ALint) buffer->bits; break;
        case AL_CHANNELS: *values = (ALint) buffer->channels; break;
        default: set_al_error(ctx, AL_INVALID_ENUM); break;
    }
}
ENTRYPOINTVOID(alGetBufferiv,(ALuint name, ALenum param, ALint *values),(name,param,values))

/* end of mojoal.c ... */

