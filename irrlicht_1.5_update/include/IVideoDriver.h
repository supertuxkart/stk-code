// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_I_VIDEO_DRIVER_H_INCLUDED__
#define __IRR_I_VIDEO_DRIVER_H_INCLUDED__

#include "rect.h"
#include "SColor.h"
#include "ITexture.h"
#include "irrArray.h"
#include "matrix4.h"
#include "plane3d.h"
#include "dimension2d.h"
#include "position2d.h"
#include "SMaterial.h"
#include "IMeshBuffer.h"
#include "triangle3d.h"
#include "EDriverTypes.h"
#include "EDriverFeatures.h"

namespace irr
{
namespace io
{
	class IAttributes;
	class IReadFile;
} // end namespace io
namespace scene
{
	class IMeshBuffer;
	class IMeshManipulator;
} // end namespace scene

namespace video
{
	struct S3DVertex;
	struct S3DVertex2TCoords;
	struct S3DVertexTangents;
	struct SLight;
	struct SExposedVideoData;
	class IImageLoader;
	class IImageWriter;
	class IMaterialRenderer;
	class IGPUProgrammingServices;

	//! enumeration for geometry transformation states
	enum E_TRANSFORMATION_STATE
	{
		//! View transformation
		ETS_VIEW = 0,
		//! World transformation
		ETS_WORLD,
		//! Projection transformation
		ETS_PROJECTION,
		//! Texture transformation
		ETS_TEXTURE_0,
		//! Texture transformation
		ETS_TEXTURE_1,
		//! Texture transformation
		ETS_TEXTURE_2,
		//! Texture transformation
		ETS_TEXTURE_3,
		//! Not used
		ETS_COUNT
	};

	enum E_LOST_RESSOURCE
	{
		//! The whole device/driver is lost
		ELR_DEVICE = 1,
		//! All texture are lost, rare problem
		ELR_TEXTURES = 2,
		//! The Render Target Textures are lost, typical problem for D3D
		ELR_RTTS = 4,
		//! The HW buffers are lost, will be recreated automatically, but might require some more time this frame
		ELR_HW_BUFFERS = 8
	};

	//! Interface to driver which is able to perform 2d and 3d graphics functions.
	/** This interface is one of the most important interfaces of
	the Irrlicht Engine: All rendering and texture manipulation is done with
	this interface. You are able to use the Irrlicht Engine by only
	invoking methods of this interface if you like to, although the
	irr::scene::ISceneManager interface provides a lot of powerful classes
	and methods to make the programmer's life easier.
	*/
	class IVideoDriver : public virtual IReferenceCounted
	{
	public:

		//! Applications must call this method before performing any rendering.
		/** This method can clear the back- and the z-buffer.
		\param backBuffer Specifies if the back buffer should be
		cleared, which means that the screen is filled with the color
		specified. If this parameter is false, the back buffer will
		not be cleared and the color parameter is ignored.
		\param zBuffer Specifies if the depth buffer (z buffer) should
		be cleared. It is not nesesarry to do so if only 2d drawing is
		used.
		\param color The color used for back buffer clearing
		\param windowId Handle of another window, if you want the
		bitmap to be displayed on another window. If this is null,
		everything will be displayed in the default window.
		Note: This feature is not fully implemented for all devices.
		\param sourceRect Pointer to a rectangle defining the source
		rectangle of the area to be presented. Set to null to present
		everything. Note: not implemented in all devices.
		\return False if failed. */
		virtual bool beginScene(bool backBuffer=true, bool zBuffer=true,
				SColor color=SColor(255,0,0,0),
				void* windowId=0,
				core::rect<s32>* sourceRect=0) = 0;

		//! Presents the rendered image to the screen.
		/** Applications must call this method after performing any
		rendering.
		\return False if failed and true if succeeded. */
		virtual bool endScene() = 0;

		//! Queries the features of the driver.
		/** Returns true if a feature is available
		\param feature Feature to query.
		\return True if the feature is available, false if not. */
		virtual bool queryFeature(E_VIDEO_DRIVER_FEATURE feature) const = 0;

		//! Disable a feature of the driver.
		/** Can also be used to enable the features again. It is not
		possible to enable unsupported features this way, though.
		\param feature Feature to disable.
		\param flag When true the feature is disabled, otherwise it is enabled. */
		virtual void disableFeature(E_VIDEO_DRIVER_FEATURE feature, bool flag=true) =0;

		//! Check if the driver was recently reset.
		/** For d3d devices you will need to recreate the RTTs if the
		driver was reset. Should be queried right after beginScene().
		*/
		virtual bool checkDriverReset() =0;

		//! Sets transformation matrices.
		/** \param state Transformation type to be set, e.g. view,
		world, or projection.
		\param mat Matrix describing the transformation. */
		virtual void setTransform(E_TRANSFORMATION_STATE state, const core::matrix4& mat) = 0;

		//! Returns the transformation set by setTransform
		/** \param state Transformation type to query
		\return Matrix describing the transformation. */
		virtual const core::matrix4& getTransform(E_TRANSFORMATION_STATE state) const = 0;

		//! Sets a material.
		/** All 3d drawing functions will draw geometry using this material thereafter.
		\param material: Material to be used from now on. */
		virtual void setMaterial(const SMaterial& material) = 0;

		//! Get access to a named texture.
		/** Loads the texture from disk if it is not
		already loaded and generates mipmap levels if desired.
		Texture loading can be influenced using the
		setTextureCreationFlag() method. The texture can be in several
		imageformats, such as BMP, JPG, TGA, PCX, PNG, and PSD.
		\param filename Filename of the texture to be loaded.
		\return Pointer to the texture, or 0 if the texture
		could not be loaded. This pointer should not be dropped. See
		IReferenceCounted::drop() for more information. */
		virtual ITexture* getTexture(const c8* filename) = 0;

		//! Get access to a named texture.
		/** Loads the texture from disk if it is not
		already loaded and generates mipmap levels if desired.
		Texture loading can be influenced using the
		setTextureCreationFlag() method. The texture can be in several
		imageformats, such as BMP, JPG, TGA, PCX, PNG, and PSD.
		\param filename Filename of the texture to be loaded.
		\return Pointer to the texture, or 0 if the texture
		could not be loaded. This pointer should not be dropped. See
		IReferenceCounted::drop() for more information. */
		virtual ITexture* getTexture(const core::stringc& filename) = 0;

		//! Get access to a named texture.
		/** Loads the texture from disk if it is not
		already loaded and generates mipmap levels if desired.
		Texture loading can be influenced using the
		setTextureCreationFlag() method. The texture can be in several
		imageformats, such as BMP, JPG, TGA, PCX, PNG, and PSD.
		\param file Pointer to an already opened file.
		\return Pointer to the texture, or 0 if the texture
		could not be loaded. This pointer should not be dropped. See
		IReferenceCounted::drop() for more information. */
		virtual ITexture* getTexture(io::IReadFile* file) = 0;

		//! Returns a texture by index
		/** \param index: Index of the texture, must be smaller than
		getTextureCount() Please note that this index might change when
		adding or removing textures
		\return Pointer to the texture, or 0 if the texture was not
		set or index is out of bounds. This pointer should not be
		dropped. See IReferenceCounted::drop() for more information. */
		virtual ITexture* getTextureByIndex(u32 index) = 0;

		//! Returns amount of textures currently loaded
		/** \return Amount of textures currently loaded */
		virtual u32 getTextureCount() const = 0;

		//! Renames a texture
		/** \param texture Pointer to the texture to rename.
		\param newName New name for the texture. This should be a unique name. */
		virtual void renameTexture(ITexture* texture, const c8* newName) = 0;

		//! Creates an empty texture of specified size.
		/** \param size: Size of the texture.
		\param name A name for the texture. Later calls to
		getTexture() with this name will return this texture
		\param format Desired color format of the texture. Please note
		that the driver may choose to create the texture in another
		color format.
		\return Pointer to the newly created texture. This pointer
		should not be dropped. See IReferenceCounted::drop() for more
		information. */
		virtual ITexture* addTexture(const core::dimension2d<s32>& size,
			const c8* name, ECOLOR_FORMAT format = ECF_A8R8G8B8) = 0;

		//! Creates a texture from an IImage.
		/** \param name A name for the texture. Later calls of
		getTexture() with this name will return this texture
		\param image Image the texture is created from.
		\return Pointer to the newly created texture. This pointer
		should not be dropped. See IReferenceCounted::drop() for more
		information. */
		virtual ITexture* addTexture(const c8* name, IImage* image) = 0;

		//! Adds a new render target texture to the texture cache.
		/** \param size Size of the texture, in pixels. Width and
		height should be a power of two (e.g. 64, 128, 256, 512, ...)
		and it should not be bigger than the backbuffer, because it
		shares the zbuffer with the screen buffer.
		\param name An optional name for the RTT.
		\return Pointer to the created texture or 0 if the texture
		could not be created. This pointer should not be dropped. See
		IReferenceCounted::drop() for more information. */
		virtual ITexture* addRenderTargetTexture(const core::dimension2d<s32>& size,
				const c8* name=0) =0;

		//! Adds a new render target texture
		/** \deprecated use addRenderTargetTexture instead. */
		virtual ITexture* createRenderTargetTexture(const core::dimension2d<s32>& size,
				const c8* name=0) =0;

		//! Removes a texture from the texture cache and deletes it.
		/** This method can free a lot of memory!
		Please note that after calling this, the pointer to the
		ITexture may no longer be valid, if it was not grabbed before
		by other parts of the engine for storing it longer. So it is a
		good idea to set all materials which are using this texture to
		0 or another texture first.
		\param texture Texture to delete from the engine cache. */
		virtual void removeTexture(ITexture* texture) = 0;

		//! Removes all textures from the texture cache and deletes them.
		/** This method can free a lot of memory!
		Please note that after calling this, the pointer to the
		ITexture may no longer be valid, if it was not grabbed before
		by other parts of the engine for storing it longer. So it is a
		good idea to set all materials which are using this texture to
		0 or another texture first. */
		virtual void removeAllTextures() = 0;

		//! Remove hardware buffer
		virtual void removeHardwareBuffer(const scene::IMeshBuffer* mb) = 0;

		//! Remove all hardware buffers
		virtual void removeAllHardwareBuffers() = 0;

		//! Creates a 1bit alpha channel of the texture based of an color key.
		/** This makes the texture transparent at the regions where
		this color key can be found when using for example draw2DImage
		with useAlphachannel==true.
		\param texture Texture whose alpha channel is modified.
		\param color Color key color. Every pixel with this color will
		become transparent as described above. Please note that the
		colors of a texture may be converted when loading it, so the
		color values may not be exactly the same in the engine and for
		example in picture edit programs. To avoid this problem, you
		could use the makeColorKeyTexture method, which takes the
		position of a pixel instead a color value. */
		virtual void makeColorKeyTexture(video::ITexture* texture, video::SColor color) const = 0;

		//! Creates a 1bit alpha channel of the texture based of an color key position.
		/** This makes the texture transparent at the regions where
		this color key can be found when using for example draw2DImage
		with useAlphachannel==true.
		\param texture Texture whose alpha channel is modified.
		\param colorKeyPixelPos Position of a pixel with the color key
		color. Every pixel with this color will become transparent as
		described above. */
		virtual void makeColorKeyTexture(video::ITexture* texture,
			core::position2d<s32> colorKeyPixelPos) const = 0;

		//! Creates a normal map from a height map texture.
		/** If the target texture has 32 bit, the height value is
		stored in the alpha component of the texture as addition. This
		value is used by the video::EMT_PARALLAX_MAP_SOLID material and
		similar materials.
		\param texture Texture whose alpha channel is modified.
		\param amplitude Constant value by which the height
		information is multiplied.*/
		virtual void makeNormalMapTexture(video::ITexture* texture, f32 amplitude=1.0f) const = 0;

		//! Sets a new render target.
		/** This will only work if the driver supports the
		EVDF_RENDER_TO_TARGET feature, which can be queried with
		queryFeature(). Usually, rendering to textures is done in this
		way:
		\code
		// create render target
		ITexture* target = driver->addRenderTargetTexture(core::dimension2d<s32>(128,128), "rtt1");

		// ...

		driver->setRenderTarget(target); // set render target
		// .. draw stuff here
		driver->setRenderTarget(0); // set previous render target
		\endcode
		Please note that you cannot render 3D or 2D geometry with a
		render target as texture on it when you are rendering the scene
		into this render target at the same time. It is usually only
		possible to render into a texture between the
		IVideoDriver::beginScene() and endScene() method calls.
		\param texture New render target. Must be a texture created with
		IVideoDriver::addRenderTargetTexture(). If set to 0, it sets
		the previous render target which was set before the last
		setRenderTarget() call.
		\param clearBackBuffer Clears the backbuffer of the render
		target with the color parameter
		\param clearZBuffer Clears the zBuffer of the rendertarget.
		Note that because the frame buffer may share the zbuffer with
		the rendertarget, its zbuffer might be partially cleared too
		by this.
		\param color The background color for the render target.
		\return True if sucessful and false if not. */
		virtual bool setRenderTarget(video::ITexture* texture,
			bool clearBackBuffer=true, bool clearZBuffer=true,
			SColor color=video::SColor(0,0,0,0)) = 0;

		//! Sets a new viewport.
		/** Every rendering operation is done into this new area.
		\param area: Rectangle defining the new area of rendering
		operations. */
		virtual void setViewPort(const core::rect<s32>& area) = 0;

		//! Gets the area of the current viewport.
		/** \return Rectangle of the current viewport. */
		virtual const core::rect<s32>& getViewPort() const = 0;

		//! Draws a vertex primitive list
		/** Note that there may be at maximum 65536 vertices, because
		the index list is an array of 16 bit values each with a maximum
		value of 65536. If there are more than 65536 vertices in the
		list, results of this operation are not defined.
		\param vertices Pointer to array of vertices.
		\param vertexCount Amount of vertices in the array.
		\param indexList Pointer to array of indices.
		\param primCount Amount of Primitives
		\param vType Vertex type, e.g. EVT_STANDARD for S3DVertex.
		\param pType Primitive type, e.g. EPT_TRIANGLE_FAN for a triangle fan. */
		virtual void drawVertexPrimitiveList(const void* vertices, u32 vertexCount,
				const void* indexList, u32 primCount,
				E_VERTEX_TYPE vType, scene::E_PRIMITIVE_TYPE pType, E_INDEX_TYPE iType) = 0;

		//! Draws an indexed triangle list.
		/** Note that there may be at maximum 65536 vertices, because
		the index list is an array of 16 bit values each with a maximum
		value of 65536. If there are more than 65536 vertices in the
		list, results of this operation are not defined.
		\param vertices Pointer to array of vertices.
		\param vertexCount Amount of vertices in the array.
		\param indexList Pointer to array of indices.
		\param triangleCount Amount of Triangles. Usually amount of indices / 3. */
		virtual void drawIndexedTriangleList(const S3DVertex* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount) = 0;

		//! Draws an indexed triangle list.
		/** Note that there may be at maximum 65536 vertices, because
		the index list is an array of 16 bit values each with a maximum
		value of 65536. If there are more than 65536 vertices in the
		list, results of this operation are not defined.
		\param vertices Pointer to array of vertices.
		\param vertexCount Amount of vertices in the array.
		\param indexList Pointer to array of indices.
		\param triangleCount Amount of Triangles. Usually amount of indices / 3. */
		virtual void drawIndexedTriangleList(const S3DVertex2TCoords* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount) = 0;

		//! Draws an indexed triangle list.
		/** Note that there may be at maximum 65536 vertices, because
		the index list is an array of 16 bit values each with a maximum
		value of 65536. If there are more than 65536 vertices in the
		list, results of this operation are not defined.
		\param vertices Pointer to array of vertices.
		\param vertexCount Amount of vertices in the array.
		\param indexList Pointer to array of indices.
		\param triangleCount Amount of Triangles. Usually amount of indices / 3. */
		virtual void drawIndexedTriangleList(const S3DVertexTangents* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount) = 0;

		//! Draws an indexed triangle fan.
		/** Note that there may be at maximum 65536 vertices, because
		the index list is an array of 16 bit values each with a maximum
		value of 65536. If there are more than 65536 vertices in the
		list, results of this operation are not defined.
		\param vertices Pointer to array of vertices.
		\param vertexCount Amount of vertices in the array.
		\param indexList Pointer to array of indices.
		\param triangleCount Amount of Triangles. Usually amount of indices - 2. */
		virtual void drawIndexedTriangleFan(const S3DVertex* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount) = 0;

		//! Draws an indexed triangle fan.
		/** Note that there may be at maximum 65536 vertices, because
		the index list is an array of 16 bit values each with a maximum
		value of 65536. If there are more than 65536 vertices in the
		list, results of this operation are not defined.
		\param vertices Pointer to array of vertices.
		\param vertexCount Amount of vertices in the array.
		\param indexList Pointer to array of indices.
		\param triangleCount Amount of Triangles. Usually amount of indices - 2. */
		virtual void drawIndexedTriangleFan(const S3DVertex2TCoords* vertices,
			u32 vertexCount, const u16* indexList, u32 triangleCount) = 0;

		//! Draws a 3d line.
		/** For some implementations, this method simply calls
		drawIndexedTriangles for some triangles.
		Note that the line is drawn using the current transformation
		matrix and material. So if you need to draw the 3D line
		independently of the current transformation, use
		\code
		driver->setMaterial(unlitMaterial);
		driver->setTransform(video::ETS_WORLD, core::matrix4());
		\endcode
		for some properly set up material before drawing the line.
		Some drivers support line thickness set in the material.
		\param start Start of the 3d line.
		\param end End of the 3d line.
		\param color Color of the line. */
		virtual void draw3DLine(const core::vector3df& start,
			const core::vector3df& end, SColor color = SColor(255,255,255,255)) = 0;

		//! Draws a 3d triangle.
		/** This method calls drawIndexedTriangles for some triangles.
		This method works with all drivers because it simply calls
		drawIndexedTriangleList but it is hence not very fast.
		Note that the triangle is drawn using the current
		transformation matrix and material. So if you need to draw it
		independently of the current transformation, use
		\code
		driver->setMaterial(unlitMaterial);
		driver->setTransform(video::ETS_WORLD, core::matrix4());
		\endcode
		for some properly set up material before drawing the triangle.
		\param triangle The triangle to draw.
		\param color Color of the line. */
		virtual void draw3DTriangle(const core::triangle3df& triangle,
			SColor color = SColor(255,255,255,255)) = 0;

		//! Draws a 3d axis aligned box.
		/** This method simply calls draw3DLine for the edges of the
		box. Note that the box is drawn using the current transformation
		matrix and material. So if you need to draw it independently of
		the current transformation, use
		\code
		driver->setMaterial(unlitMaterial);
		driver->setTransform(video::ETS_WORLD, core::matrix4());
		\endcode
		for some properly set up material before drawing the box.
		\param box The axis aligned box to draw
		\param color Color to use while drawing the box. */
		virtual void draw3DBox(const core::aabbox3d<f32>& box,
			SColor color = SColor(255,255,255,255)) = 0;

		//! Draws a 2d image without any special effects
		/** \param texture Pointer to texture to use.
		\param destPos Upper left 2d destination position where the
		image will be drawn. */
		virtual void draw2DImage(const video::ITexture* texture,
			const core::position2d<s32>& destPos) = 0;

		//! Draws a 2d image using a color
		/** (if color is other than
		Color(255,255,255,255)) and the alpha channel of the texture.
		\param texture Texture to be drawn.
		\param destPos Upper left 2d destination position where the
		image will be drawn.
		\param sourceRect Source rectangle in the image.
		\param clipRect Pointer to rectangle on the screen where the
		image is clipped to.
		If this pointer is NULL the image is not clipped.
		\param color Color with which the image is drawn. If the color
		equals Color(255,255,255,255) it is ignored. Note that the
		alpha component is used: If alpha is other than 255, the image
		will be transparent.
		\param useAlphaChannelOfTexture: If true, the alpha channel of
		the texture is used to draw the image.*/
		virtual void draw2DImage(const video::ITexture* texture, const core::position2d<s32>& destPos,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			SColor color=SColor(255,255,255,255), bool useAlphaChannelOfTexture=false) = 0;

		//! Draws a set of 2d images, using a color and the alpha channel of the texture.
		/** The images are drawn beginning at pos and concatenated in
		one line. All drawings are clipped against clipRect (if != 0).
		The subtextures are defined by the array of sourceRects and are
		chosen by the indices given.
		\param texture Texture to be drawn.
		\param pos Upper left 2d destination position where the image
		will be drawn.
		\param sourceRects Source rectangles of the image.
		\param indices List of indices which choose the actual
		rectangle used each time.
		\param kerningWidth Offset to Position on X
		\param clipRect Pointer to rectangle on the screen where the
		image is clipped to.
		If this pointer is 0 then the image is not clipped.
		\param color Color with which the image is drawn.
		Note that the alpha component is used. If alpha is other than
		255, the image will be transparent.
		\param useAlphaChannelOfTexture: If true, the alpha channel of
		the texture is used to draw the image. */
		virtual void draw2DImage(const video::ITexture* texture,
				const core::position2d<s32>& pos,
				const core::array<core::rect<s32> >& sourceRects,
				const core::array<s32>& indices,
				s32 kerningWidth=0,
				const core::rect<s32>* clipRect=0,
				SColor color=SColor(255,255,255,255),
				bool useAlphaChannelOfTexture=false) = 0;

		//! Draws a part of the texture into the rectangle. Note that colors must be an array of 4 colors if used.
		/** Suggested and first implemented by zola.
		\param texture The texture to draw from
		\param destRect The rectangle to draw into
		\param sourceRect The rectangle denoting a part of the texture
		\param clipRect Clips the destination rectangle (may be 0)
		\param colors Array of 4 colors denoting the color values of
		the corners of the destRect
		\param useAlphaChannelOfTexture True if alpha channel will be
		blended. */
		virtual void draw2DImage(const video::ITexture* texture, const core::rect<s32>& destRect,
			const core::rect<s32>& sourceRect, const core::rect<s32>* clipRect = 0,
			const video::SColor * const colors=0, bool useAlphaChannelOfTexture=false) = 0;

		//! Draws a 2d rectangle.
		/** \param color Color of the rectangle to draw. The alpha
		component will not be ignored and specifies how transparent the
		rectangle will be.
		\param pos Position of the rectangle.
		\param clip Pointer to rectangle against which the rectangle
		will be clipped. If the pointer is null, no clipping will be
		performed. */
		virtual void draw2DRectangle(SColor color, const core::rect<s32>& pos,
			const core::rect<s32>* clip = 0) = 0;

		//! Draws an 2d rectangle with a gradient.
		/** \param colorLeftUp Color of the upper left corner to draw.
		The alpha component will not be ignored and specifies how
		transparent the rectangle will be.
		\param colorRightUp Color of the upper right corner to draw.
		The alpha component will not be ignored and specifies how
		transparent the rectangle will be.
		\param colorLeftDown Color of the lower left corner to draw.
		The alpha component will not be ignored and specifies how
		transparent the rectangle will be.
		\param colorRightDown Color of the lower right corner to draw.
		The alpha component will not be ignored and specifies how
		transparent the rectangle will be.
		\param pos Position of the rectangle.
		\param clip Pointer to rectangle against which the rectangle
		will be clipped. If the pointer is null, no clipping will be
		performed. */
		virtual void draw2DRectangle(const core::rect<s32>& pos,
				SColor colorLeftUp, SColor colorRightUp,
				SColor colorLeftDown, SColor colorRightDown,
				const core::rect<s32>* clip = 0) = 0;

		//! Draws a 2d line.
		/** \param start: Screen coordinates of the start of the line
		in pixels.
		\param end: Screen coordinates of the start of the line in
		pixels.
		\param color: Color of the line to draw. */
		virtual void draw2DLine(const core::position2d<s32>& start,
					const core::position2d<s32>& end,
					SColor color=SColor(255,255,255,255)) = 0;

		//! Draws a pixel.
		/** \param position: the position of the pixel.
		\param color: Color of the pixel to draw. */
		virtual void drawPixel(u32 x, u32 y, const SColor & color) = 0; 

		//! Draws a non filled concyclic regular 2d polyon.
		/** This method can be used to draw circles, but also
		triangles, tetragons, pentagons, hexagons, heptagons, octagons,
		enneagons, decagons, hendecagons, dodecagon, triskaidecagons,
		etc. I think you'll got it now. And all this by simply
		specifying the vertex count. Welcome to the wonders of
		geometry.
		\param center Position of center of circle (pixels).
		\param radius Radius of circle in pixels.
		\param color Color of the circle.
		\param vertexCount Amount of vertices of the polygon. Specify 2
		to draw a line, 3 to draw a triangle, 4 for tetragons and a lot
		(>10) for nearly a circle. */
		virtual void draw2DPolygon(core::position2d<s32> center,
				f32 radius,
				video::SColor color=SColor(100,255,255,255),
				s32 vertexCount=10) = 0;
               
		//! Draws a filled convex 2d polyon.
		/** This method can be used to draw (approximated) circles
                etc., but also any convex 2d polygon.
                \param vertices The vertices of the polygon.
                \param colors The color for each vertex. If less colors are
                specified than there are vertices, color[0] is used!
                So to fill a polygon with one colour, you only have to
                specify a 1-element array. If NULL, no colour values
                are set.
                \param texture A texture to apply to the polygon (optional).
                \param coordinates Texture coordinates for the polygon (only
                used if a texture is specified). 
                \param useAlphaChannelOfTexture True if alpha channels
                of the texture should be used. */
                virtual void draw2DPolygon(const core::array<core::vector2df> &vertices,
                           const core::array<video::SColor> *colors=NULL,
                           const video::ITexture *texture=NULL,
                           bool useAlphaChannelOfTexture=false,
                           const core::array<core::vector2di> *coordinates=NULL) {};
#define IRRLICHT_HAS_SUPERTUXKART_POLYGON 1

		//! Draws a shadow volume into the stencil buffer.
		/** To draw a stencil shadow, do this: First, draw all geometry.
		Then use this method, to draw the shadow volume. Then, use
		IVideoDriver::drawStencilShadow() to visualize the shadow.
		Please note that the code for the opengl version of the method
		is based on free code sent in by Philipp Dortmann, lots of
		thanks go to him!
		\param triangles Pointer to array of 3d vectors, specifying the
		shadow volume.
		\param count Amount of triangles in the array.
		\param zfail If set to true, zfail method is used, otherwise
		zpass. */
		virtual void drawStencilShadowVolume(const core::vector3df* triangles, s32 count, bool zfail=true) = 0;

		//! Fills the stencil shadow with color.
		/** After the shadow volume has been drawn into the stencil
		buffer using IVideoDriver::drawStencilShadowVolume(), use this
		to draw the color of the shadow.
		Please note that the code for the opengl version of the method
		is based on free code sent in by Philipp Dortmann, lots of
		thanks go to him!
		\param clearStencilBuffer Set this to false, if you want to
		draw every shadow with the same color, and only want to call
		drawStencilShadow() once after all shadow volumes have been
		drawn. Set this to true, if you want to paint every shadow with
		its own color.
		\param leftUpEdge Color of the shadow in the upper left corner
		of screen.
		\param rightUpEdge Color of the shadow in the upper right
		corner of screen.
		\param leftDownEdge Color of the shadow in the lower left
		corner of screen.
		\param rightDownEdge Color of the shadow in the lower right
		corner of screen. */
		virtual void drawStencilShadow(bool clearStencilBuffer=false,
			video::SColor leftUpEdge = video::SColor(255,0,0,0),
			video::SColor rightUpEdge = video::SColor(255,0,0,0),
			video::SColor leftDownEdge = video::SColor(255,0,0,0),
			video::SColor rightDownEdge = video::SColor(255,0,0,0)) = 0;

		//! Draws a mesh buffer
		/** \param mb: Buffer to draw; */
		virtual void drawMeshBuffer( const scene::IMeshBuffer* mb) = 0;

		//! Sets the fog mode.
		/** These are global values attached to each 3d object rendered,
		which has the fog flag enabled in its material.
		\param color Color of the fog
		\param linearFog Set this to true for linear fog, otherwise
		exponential fog is applied.
		\param start Only used in linear fog mode (linearFog=true).
		Specifies where fog starts.
		\param end Only used in linear fog mode (linearFog=true).
		Specifies where fog ends.
		\param density Only used in exponential fog mode
		(linearFog=false). Must be a value between 0 and 1.
		\param pixelFog Set this to false for vertex fog, and true if
		you want per-pixel fog.
		\param rangeFog Set this to true to enable range-based vertex
		fog. The distance from the viewer is used to compute the fog,
		not the z-coordinate. This is better, but slower. This is only
		available with D3D and vertex fog. */
		virtual void setFog(SColor color=SColor(0,255,255,255),
				bool linearFog=true, f32 start=50.0f, f32 end=100.0f,
				f32 density=0.01f,
				bool pixelFog=false, bool rangeFog=false) = 0;

		//! Get the current color format of the color buffer
		/** \return Color format of the color buffer. */
		virtual ECOLOR_FORMAT getColorFormat() const = 0;

		//! Get the size of the screen or render window.
		/** \return Size of screen or render window. */
		virtual const core::dimension2d<s32>& getScreenSize() const = 0;

		//! Get the size of the current render target
		/** This method will return the screen size if the driver
		doesn't support render to texture, or if the current render
		target is the screen.
		\return Size of render target or screen/window */
		virtual const core::dimension2d<s32>& getCurrentRenderTargetSize() const = 0;

		//! Returns current frames per second value.
		/** This value is updated approximately every 1.5 seconds and
		is only intended to provide a rough guide to the average frame
		rate. It is not suitable for use in performing timing
		calculations or framerate independent movement.
		\return Approximate amount of frames per second drawn. */
		virtual s32 getFPS() const = 0;

		//! Returns amount of primitives (mostly triangles) which were drawn in the last frame.
		/** Together with getFPS() very useful method for statistics.
		\param mode Defines if the primitives drawn are accumulated or
		counted per frame.
		\return Amount of primitives drawn in the last frame. */
		virtual u32 getPrimitiveCountDrawn( u32 mode = 0 ) const = 0;

		//! Deletes all dynamic lights which were previously added with addDynamicLight().
		virtual void deleteAllDynamicLights() = 0;

		//! Adds a dynamic light.
		/** \param light Data specifying the dynamic light. */
		virtual void addDynamicLight(const SLight& light) = 0;

		//! Returns the maximal amount of dynamic lights the device can handle
		/** \return Maximal amount of dynamic lights. */
		virtual u32 getMaximalDynamicLightAmount() const = 0;

		//! Returns amount of dynamic lights currently set
		/** \return Amount of dynamic lights currently set */
		virtual u32 getDynamicLightCount() const = 0;

		//! Returns light data which was previously set by IVideoDriver::addDynamicLight().
		/** \param idx Zero based index of the light. Must be 0 or
		greater and smaller than IVideoDriver()::getDynamicLightCount.
		\return Light data. */
		virtual const SLight& getDynamicLight(u32 idx) const = 0;

		//! Gets name of this video driver.
		/** \return Returns the name of the video driver, e.g. in case
		of the Direct3D8 driver, it would return "Direct3D 8.1". */
		virtual const wchar_t* getName() const = 0;

		//! Adds an external image loader to the engine.
		/** This is useful if the Irrlicht Engine should be able to load
		textures of currently unsupported file formats (e.g. gif). The
		IImageLoader only needs to be implemented for loading this file
		format. A pointer to the implementation can be passed to the
		engine using this method.
		\param loader Pointer to the external loader created. */
		virtual void addExternalImageLoader(IImageLoader* loader) = 0;

		//! Adds an external image writer to the engine.
		/** This is useful if the Irrlicht Engine should be able to
		write textures of currently unsupported file formats (e.g
		.gif). The IImageWriter only needs to be implemented for
		writing this file format. A pointer to the implementation can
		be passed to the engine using this method.
		\param writer: Pointer to the external writer created. */
		virtual void addExternalImageWriter(IImageWriter* writer) = 0;

		//! Returns the maximum amount of primitives
		/** (mostly vertices) which the device is able to render with
		one drawIndexedTriangleList call.
		\return Maximum amount of primitives. */
		virtual u32 getMaximalPrimitiveCount() const = 0;

		//! Enables or disables a texture creation flag.
		/** These flags define how textures should be created. By
		changing this value, you can influence for example the speed of
		rendering a lot. But please note that the video drivers take
		this value only as recommendation. It could happen that you
		enable the ETCF_ALWAYS_16_BIT mode, but the driver still creates
		32 bit textures.
		\param flag Texture creation flag.
		\param enabled Specifies if the given flag should be enabled or
		disabled. */
		virtual void setTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag, bool enabled) = 0;

		//! Returns if a texture creation flag is enabled or disabled.
		/** You can change this value using setTextureCreationMode().
		\param flag Texture creation flag.
		\return The current texture creation mode. */
		virtual bool getTextureCreationFlag(E_TEXTURE_CREATION_FLAG flag) const = 0;

		//! Creates a software image from a file.
		/** No hardware texture will be created for this image. This
		method is useful for example if you want to read a heightmap
		for a terrain renderer.
		\param filename Name of the file from which the image is
		created.
		\return The created image.
		If you no longer need the image, you should call IImage::drop().
		See IReferenceCounted::drop() for more information. */
		virtual IImage* createImageFromFile(const c8* filename) = 0;

		//! Creates a software image from a file.
		/** No hardware texture will be created for this image. This
		method is useful for example if you want to read a heightmap
		for a terrain renderer.
		\param file File from which the image is created.
		\return The created image.
		If you no longer need the image, you should call IImage::drop().
		See IReferenceCounted::drop() for more information. */
		virtual IImage* createImageFromFile(io::IReadFile* file) = 0;

		//! Writes the provided image to a file.
		/** Requires that there is a suitable image writer registered
		for writing the image.
		\param image Image to write.
		\param filename Name of the file to write.
		\param param Control parameter for the backend (e.g. compression
		level).
		\return True on successful write. */
		virtual bool writeImageToFile(IImage* image, const c8* filename, u32 param = 0) = 0;

		//! Creates a software image from a byte array.
		/** No hardware texture will be created for this image. This
		method is useful for example if you want to read a heightmap
		for a terrain renderer.
		\param format Desired color format of the texture
		\param size Desired size of the image
		\param data A byte array with pixel color information
		\param ownForeignMemory If true, the image will use the data
		pointer directly and own it afterwards. If false, the memory
		will by copied internally.
		\param deleteMemory Whether the memory is deallocated upon
		destruction.
		\return The created image.
		If you no longer need the image, you should call IImage::drop().
		See IReferenceCounted::drop() for more information. */
		virtual IImage* createImageFromData(ECOLOR_FORMAT format,
			const core::dimension2d<s32>& size, void *data,
			bool ownForeignMemory=false,
			bool deleteMemory = true) = 0;

		//! Creates an empty software image.
		/**
		\param format Desired color format of the image.
		\param size Size of the image to create.
		\return The created image.
		If you no longer need the image, you should call IImage::drop().
		See IReferenceCounted::drop() for more information. */
		virtual IImage* createImage(ECOLOR_FORMAT format, const core::dimension2d<s32>& size) =0;

		//! Creates a software image by converting it to given format from another image.
		/**
		\param format Desired color format of the image.
		\param imageToCopy Image to copy to the new image.
		\return The created image.
		If you no longer need the image, you should call IImage::drop().
		See IReferenceCounted::drop() for more information. */
		virtual IImage* createImage(ECOLOR_FORMAT format, IImage *imageToCopy) =0;

		//! Creates a software image from a part of another image.
		/**
		\param imageToCopy Image to copy the the new image in part.
		\param pos Position of rectangle to copy.
		\param size Extents of rectangle to copy.
		\return The created image.
		If you no longer need the image, you should call IImage::drop().
		See IReferenceCounted::drop() for more information. */
		virtual IImage* createImage(IImage* imageToCopy,
				const core::position2d<s32>& pos,
				const core::dimension2d<s32>& size) =0;

		//! Event handler for resize events. Only used by the engine internally.
		/** Used to notify the driver that the window was resized.
		Usually, there is no need to call this method. */
		virtual void OnResize(const core::dimension2d<s32>& size) = 0;

		//! Adds a new material renderer to the video device.
		/** Use this method to extend the VideoDriver with new material
		types. To extend the engine using this method do the following:
		Derive a class from IMaterialRenderer and override the methods
		you need. For setting the right renderstates, you can try to
		get a pointer to the real rendering device using
		IVideoDriver::getExposedVideoData(). Add your class with
		IVideoDriver::addMaterialRenderer(). To use an object being
		displayed with your new material, set the MaterialType member of
		the SMaterial struct to the value returned by this method.
		If you simply want to create a new material using vertex and/or
		pixel shaders it would be easier to use the
		video::IGPUProgrammingServices interface which you can get
		using the getGPUProgrammingServices() method.
		\param renderer A pointer to the new renderer.
		\param name Optional name for the material renderer entry.
		\return The number of the material type which can be set in
		SMaterial::MaterialType to use the renderer. -1 is returned if
		an error occured. For example if you tried to add an material
		renderer to the software renderer or the null device, which do
		not accept material renderers. */
		virtual s32 addMaterialRenderer(IMaterialRenderer* renderer, const c8* name = 0) = 0;

		//! Get access to a material renderer by index.
		/** \param idx Id of the material renderer. Can be a value of
		the E_MATERIAL_TYPE enum or a value which was returned by
		addMaterialRenderer().
		\return Pointer to material renderer or null if not existing. */
		virtual IMaterialRenderer* getMaterialRenderer(u32 idx) = 0;

		//! Get amount of currently available material renderers.
		/** \return Amount of currently available material renderers. */
		virtual u32 getMaterialRendererCount() const = 0;

		//! Get name of a material renderer
		/** This string can, e.g., be used to test if a specific
		renderer already has been registered/created, or use this
		string to store data about materials: This returned name will
		be also used when serializing materials.
		\param idx Id of the material renderer. Can be a value of the
		E_MATERIAL_TYPE enum or a value which was returned by
		addMaterialRenderer().
		\return String with the name of the renderer, or 0 if not
		exisiting */
		virtual const c8* getMaterialRendererName(u32 idx) const = 0;

		//! Sets the name of a material renderer.
		/** Will have no effect on built-in material renderers.
		\param idx: Id of the material renderer. Can be a value of the
		E_MATERIAL_TYPE enum or a value which was returned by
		addMaterialRenderer().
		\param name: New name of the material renderer. */
		virtual void setMaterialRendererName(s32 idx, const c8* name) = 0;

		//! Creates material attributes list from a material
		/** This method is useful for serialization and more.
		Please note that the video driver will use the material
		renderer names from getMaterialRendererName() to write out the
		material type name, so they should be set before.
		\param material The material to serialize.
		\return The io::IAttributes container holding the material
		properties. */
		virtual io::IAttributes* createAttributesFromMaterial(const video::SMaterial& material) = 0;

		//! Fills an SMaterial structure from attributes.
		/** Please note that for setting material types of the
		material, the video driver will need to query the material
		renderers for their names, so all non built-in materials must
		have been created before calling this method.
		\param outMaterial The material to set the properties for.
		\param attributes The attributes to read from. */
		virtual void fillMaterialStructureFromAttributes(video::SMaterial& outMaterial, io::IAttributes* attributes) = 0;

		//! Returns driver and operating system specific data about the IVideoDriver.
		/** This method should only be used if the engine should be
		extended without having to modify the source of the engine.
		\return Collection of device dependent pointers. */
		virtual const SExposedVideoData& getExposedVideoData() = 0;

		//! Get type of video driver
		/** \return Type of driver. */
		virtual E_DRIVER_TYPE getDriverType() const = 0;

		//! Gets the IGPUProgrammingServices interface.
		/** \return Pointer to the IGPUProgrammingServices. Returns 0
		if the video driver does not support this. For example the
		Software driver and the Null driver will always return 0. */
		virtual IGPUProgrammingServices* getGPUProgrammingServices() = 0;

		//! Returns a pointer to the mesh manipulator.
		virtual scene::IMeshManipulator* getMeshManipulator() = 0;

		//! Clears the ZBuffer.
		/** Note that you usually need not to call this method, as it
		is automatically done in IVideoDriver::beginScene() or
		IVideoDriver::setRenderTarget() if you enable zBuffer. But if
		you have to render some special things, you can clear the
		zbuffer during the rendering process with this method any time.
		*/
		virtual void clearZBuffer() = 0;

		//! Make a screenshot of the last rendered frame.
		/** \return An image created from the last rendered frame. */
		virtual IImage* createScreenShot() = 0;

		//! Check if the image is already loaded.
		/** Works similar to getTexture(), but does not load the texture
		if it is not currently loaded.
		\param filename Name of the texture.
		\return Pointer to loaded texture, or 0 if not found. */
		virtual video::ITexture* findTexture(const c8* filename) = 0;

		//! Set or unset a clipping plane.
		/** There are at least 6 clipping planes available for the user
		to set at will.
		\param index The plane index. Must be between 0 and
		MaxUserClipPlanes.
		\param plane The plane itself.
		\param enable If true, enable the clipping plane else disable
		it.
		\return True if the clipping plane is usable. */
		virtual bool setClipPlane(u32 index, const core::plane3df& plane, bool enable=false) = 0;

		//! Enable or disable a clipping plane.
		/** There are at least 6 clipping planes available for the user
		to set at will.
		\param index The plane index. Must be between 0 and
		MaxUserClipPlanes.
		\param enable If true, enable the clipping plane else disable
		it. */
		virtual void enableClipPlane(u32 index, bool enable) = 0;

		//! Returns the graphics card vendor name.
		virtual core::stringc getVendorInfo() = 0;

		//! Only used by the engine internally.
		/** The ambient color is set in the scene manager, see
		scene::ISceneManager::setAmbientLight().
		\param color New color of the ambient light. */
		virtual void setAmbientLight(const SColorf& color) = 0;

		//! Only used by the engine internally.
		/** Passes the global material flag AllowZWriteOnTransparent.
		Use the SceneManager attribute to set this value from your app.
		\param flag Default behavior is to disable ZWrite, i.e. false. */
		virtual void setAllowZWriteOnTransparent(bool flag) = 0;
	};

} // end namespace video
} // end namespace irr


#endif



