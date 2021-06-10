#ifndef __VULKAN_DRIVER_INCLUDED__
#define __VULKAN_DRIVER_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_VULKAN_

#include "vulkan_wrapper.h"
#include "SDL_video.h"

#include "../source/Irrlicht/CNullDriver.h"
#include "SIrrCreationParameters.h"
#include "SColor.h"
#include <string>
#include <vector>

using namespace irr;
using namespace video;

namespace GE
{
    class GEVulkanDriver : public video::CNullDriver
    {
    public:

        //! constructor
        GEVulkanDriver(const SIrrlichtCreationParameters& params, io::IFileSystem* io, SDL_Window* window);

        //! destructor
        virtual ~GEVulkanDriver();

        //! applications must call this method before performing any rendering. returns false if failed.
        virtual bool beginScene(bool backBuffer=true, bool zBuffer=true,
                SColor color=SColor(255,0,0,0),
                const SExposedVideoData& videoData=SExposedVideoData(),
                core::rect<s32>* sourceRect=0) { return true; }

        //! applications must call this method after performing any rendering. returns false if failed.
        virtual bool endScene() { return true; }

        //! queries the features of the driver, returns true if feature is available
        virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const  { return true; }

        //! sets transformation
        virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat) {}

        //! sets a material
        virtual void setMaterial(const SMaterial& material) {}

        //! sets a render target
        virtual bool setRenderTarget(video::ITexture* texture,
            bool clearBackBuffer=true, bool clearZBuffer=true,
            SColor color=video::SColor(0,0,0,0)) { return true; }

        //! Sets multiple render targets
        virtual bool setRenderTarget(const core::array<video::IRenderTarget>& texture,
            bool clearBackBuffer=true, bool clearZBuffer=true,
            SColor color=video::SColor(0,0,0,0)) { return true; }

        //! sets a viewport
        virtual void setViewPort(const core::rect<s32>& area) {}

        //! gets the area of the current viewport
        virtual const core::rect<s32>& getViewPort() const
        {
            static core::rect<s32> unused;
            return unused;
        }

        //! updates hardware buffer if needed
        virtual bool updateHardwareBuffer(SHWBufferLink *HWBuffer) { return false; }

        //! Create hardware buffer from mesh
        virtual SHWBufferLink *createHardwareBuffer(const scene::IMeshBuffer* mb) { return NULL; }

        //! Delete hardware buffer (only some drivers can)
        virtual void deleteHardwareBuffer(SHWBufferLink *HWBuffer) {}

        //! Draw hardware buffer
        virtual void drawHardwareBuffer(SHWBufferLink *HWBuffer) {}

        //! Create occlusion query.
        /** Use node for identification and mesh for occlusion test. */
        virtual void addOcclusionQuery(scene::ISceneNode* node,
                const scene::IMesh* mesh=0) {}

        //! Remove occlusion query.
        virtual void removeOcclusionQuery(scene::ISceneNode* node) {}

        //! Run occlusion query. Draws mesh stored in query.
        /** If the mesh shall not be rendered visible, use
        overrideMaterial to disable the color and depth buffer. */
        virtual void runOcclusionQuery(scene::ISceneNode* node, bool visible=false) {}

        //! Update occlusion query. Retrieves results from GPU.
        /** If the query shall not block, set the flag to false.
        Update might not occur in this case, though */
        virtual void updateOcclusionQuery(scene::ISceneNode* node, bool block=true) {}

        //! Return query result.
        /** Return value is the number of visible pixels/fragments.
        The value is a safe approximation, i.e. can be larger then the
        actual value of pixels. */
        virtual u32 getOcclusionQueryResult(scene::ISceneNode* node) const { return 0; }

        //! draws a vertex primitive list
        virtual void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType) {}

        //! draws a vertex primitive list in 2d
        virtual void draw2DVertexPrimitiveList(const void* vertices, u32 vertexCount,
                const void* indexList, u32 primitiveCount,
                E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType,
                E_INDEX_TYPE iType) {}

        //! draws an 2d image, using a color (if color is other then Color(255,255,255,255)) and the alpha channel of the texture if wanted.
        virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
            const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
            SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false) {}

        //! Draws a part of the texture into the rectangle.
        virtual void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
            const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
            const video::SColor* const colors=0, bool useAlphaChannelOfTexture=false) {}

        //! Draws a set of 2d images, using a color and the alpha channel of the texture.
        virtual void draw2DImageBatch(const video::ITexture* texture,
                const core::array<core::position2d<s32> >& positions,
                const core::array<core::rect<s32> >& sourceRects,
                const core::rect<s32>* clipRect=0,
                SColor color=SColor(255,255,255,255),
                bool useAlphaChannelOfTexture=false) {}

        //!Draws an 2d rectangle with a gradient.
        virtual void draw2DRectangle(const core::rect<s32>& pos,
            SColor colorLeftUp, SColor colorRightUp, SColor colorLeftDown, SColor colorRightDown,
            const core::rect<s32>* clip) {}

        //! Draws a 2d line.
        virtual void draw2DLine(const core::position2d<s32>& start,
                    const core::position2d<s32>& end,
                    SColor color=SColor(255,255,255,255)) {}

        //! Draws a pixel.
        virtual void drawPixel(u32 x, u32 y, const SColor & color) {}

        //! Draws a 3d line.
        virtual void draw3DLine(const core::vector3df& start,
            const core::vector3df& end, SColor color = SColor(255,255,255,255)) {}

        //! \return Returns the name of the video driver. Example: In case of the DIRECT3D8
        //! driver, it would return "Direct3D8.1".
        virtual const wchar_t* getName() const { return L""; }

        //! deletes all dynamic lights there are
        virtual void deleteAllDynamicLights() {}

        //! adds a dynamic light, returning an index to the light
        //! \param light: the light data to use to create the light
        //! \return An index to the light, or -1 if an error occurs
        virtual s32 addDynamicLight(const SLight& light) { return -1; }

        //! Turns a dynamic light on or off
        //! \param lightIndex: the index returned by addDynamicLight
        //! \param turnOn: true to turn the light on, false to turn it off
        virtual void turnLightOn(s32 lightIndex, bool turnOn) {}

        //! returns the maximal amount of dynamic lights the device can handle
        virtual u32 getMaximalDynamicLightAmount() const { return (u32)-1; }

        //! Sets the dynamic ambient light color. The default color is
        //! (0,0,0,0) which means it is dark.
        //! \param color: New color of the ambient light.
        virtual void setAmbientLight(const SColorf& color) {}

        //! Draws a shadow volume into the stencil buffer.
        virtual void drawStencilShadowVolume(const core::array<core::vector3df>& triangles, bool zfail=true, u32 debugDataVisible=0) {}

        //! Fills the stencil shadow with color.
        virtual void drawStencilShadow(bool clearStencilBuffer=false,
            video::SColor leftUpEdge = video::SColor(0,0,0,0),
            video::SColor rightUpEdge = video::SColor(0,0,0,0),
            video::SColor leftDownEdge = video::SColor(0,0,0,0),
            video::SColor rightDownEdge = video::SColor(0,0,0,0)) {}

        //! Returns the maximum amount of primitives (mostly vertices) which
        //! the device is able to render with one drawIndexedTriangleList
        //! call.
        virtual u32 getMaximalPrimitiveCount() const { return (u32)-1; }

        //! Enables or disables a texture creation flag.
        virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled) {}

        //! Sets the fog mode.
        virtual void setFog(SColor color, E_FOG_TYPE fogType, f32 start,
            f32 end, f32 density, bool pixelFog, bool rangeFog) {}

        //! Only used by the internal engine. Used to notify the driver that
        //! the window was resized.
        virtual void OnResize(const core::dimension2d<u32>& size) {}

        //! Returns type of video driver
        virtual E_DRIVER_TYPE getDriverType() const { return video::EDT_VULKAN; }

        //! Returns the transformation set by setTransform
        virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const
        {
            static core::matrix4 unused;
            return unused;
        }

        //! Creates a render target texture.
        virtual ITexture* addRenderTargetTexture(const core::dimension2d<u32>& size,
                const io::path& name, const ECOLOR_FORMAT format = ECF_UNKNOWN, const bool useStencil = false) { return NULL; }

        //! Clears the ZBuffer.
        virtual void clearZBuffer() {}

        //! Returns an image created from the last rendered frame.
        virtual IImage* createScreenShot(video::ECOLOR_FORMAT format=video::ECF_UNKNOWN, video::E_RENDER_TARGET target=video::ERT_FRAME_BUFFER) { return NULL; }

        //! Set/unset a clipping plane.
        virtual bool setClipPlane(u32 index, const core::plane3df& plane, bool enable=false) { return true; }

        //! Enable/disable a clipping plane.
        virtual void enableClipPlane(u32 index, bool enable) {}

        //! Returns the graphics card vendor name.
        virtual core::stringc getVendorInfo()
        {
            switch (m_properties.vendorID)
            {
                case 0x1002: return "AMD";
                case 0x1010: return "ImgTec";
                case 0x106B: return "Apple";
                case 0x10DE: return "NVIDIA";
                case 0x13B5: return "ARM";
                case 0x14e4: return "Broadcom";
                case 0x5143: return "Qualcomm";
                case 0x8086: return "INTEL";
                // llvmpipe
                case 0x10005: return "Mesa";
                default: return "Unknown";
            }
        }

        //! Enable the 2d override material
        virtual void enableMaterial2D(bool enable=true) {}

        //! Check if the driver was recently reset.
        virtual bool checkDriverReset() { return false; }

        //! Get the current color format of the color buffer
        /** \return Color format of the color buffer. */
        virtual ECOLOR_FORMAT getColorFormat() const { return ECF_A8R8G8B8; }

        //! Returns the maximum texture size supported.
        virtual core::dimension2du getMaxTextureSize() const { return core::dimension2du(16384, 16384); }

        virtual void enableScissorTest(const core::rect<s32>& r) {}
        virtual void disableScissorTest() {}

    private:
        //! returns a device dependent texture from a software surface (IImage)
        //! THIS METHOD HAS TO BE OVERRIDDEN BY DERIVED DRIVERS WITH OWN TEXTURES
        virtual video::ITexture* createDeviceDependentTexture(IImage* surface, const io::path& name, void* mipmapData=0) { return NULL; }

        //! returns the current size of the screen or rendertarget
        virtual const core::dimension2d<u32>& getCurrentRenderTargetSize() const
        {
            static core::dimension2d<u32> unused;
            return unused;
        }

        //! Adds a new material renderer to the VideoDriver, based on a high level shading
        //! language.
        virtual s32 addHighLevelShaderMaterial(
            const c8* vertexShaderProgram,
            const c8* vertexShaderEntryPointName,
            E_VERTEX_SHADER_TYPE vsCompileTarget,
            const c8* pixelShaderProgram,
            const c8* pixelShaderEntryPointName,
            E_PIXEL_SHADER_TYPE psCompileTarget,
            const c8* geometryShaderProgram,
            const c8* geometryShaderEntryPointName = "main",
            E_GEOMETRY_SHADER_TYPE gsCompileTarget = EGST_GS_4_0,
            scene::E_PRIMITIVE_TYPE inType = scene::EPT_TRIANGLES,
            scene::E_PRIMITIVE_TYPE outType = scene::EPT_TRIANGLE_STRIP,
            u32 verticesOut = 0,
            IShaderConstantSetCallBack* callback = 0,
            E_MATERIAL_TYPE baseMaterial = video::EMT_SOLID,
            s32 userData = 0,
            E_GPU_SHADING_LANGUAGE shadingLang = EGSL_DEFAULT) { return 0; }

        // RAII to auto cleanup
        struct VK
        {
            VkInstance instance;
            VkSurfaceKHR surface;
            VkDevice device;
            VK()
            {
                instance = VK_NULL_HANDLE;
                surface = VK_NULL_HANDLE;
                device = VK_NULL_HANDLE;
            }
            ~VK()
            {
                if (device != VK_NULL_HANDLE)
                    vkDestroyDevice(device, NULL);
                if (surface != VK_NULL_HANDLE)
                    vkDestroySurfaceKHR(instance, surface, NULL);
                if (instance != VK_NULL_HANDLE)
                    vkDestroyInstance(instance, NULL);
            }
        };
        VK m_vk;
        VkPhysicalDevice m_physical_device;
        std::vector<const char*> m_device_extensions;
        VkSurfaceCapabilitiesKHR m_surface_capabilities;
        std::vector<VkSurfaceFormatKHR> m_surface_formats;
        std::vector<VkPresentModeKHR> m_present_modes;
        VkQueue m_graphics_queue;
        VkQueue m_present_queue;

        uint32_t m_graphics_family;
        uint32_t m_present_family;
        VkPhysicalDeviceProperties m_properties;
        VkPhysicalDeviceFeatures m_features;

        void createInstance(SDL_Window* window);
        void findPhysicalDevice();
        bool checkDeviceExtensions(VkPhysicalDevice device);
        bool findQueueFamilies(VkPhysicalDevice device, uint32_t* graphics_family, uint32_t* present_family);
        bool updateSurfaceInformation(VkPhysicalDevice device,
                                      VkSurfaceCapabilitiesKHR* surface_capabilities,
                                      std::vector<VkSurfaceFormatKHR>* surface_formats,
                                      std::vector<VkPresentModeKHR>* present_modes);
        void createDevice();
        std::string getVulkanVersionString() const;
        std::string getDriverVersionString() const;
    };

}

#endif // _IRR_COMPILE_WITH_VULKAN_
#endif // __VULKAN_DRIVER_INCLUDED__

