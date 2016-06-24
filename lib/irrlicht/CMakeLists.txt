# CMakeLists.txt for Irrlicht in STK
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/"
                    "${JPEG_INCLUDE_DIR}"
                    "${PNG_INCLUDE_DIRS}"
                    "${ZLIB_INCLUDE_DIR}"
                    "${CMAKE_CURRENT_BINARY_DIR}/../zlib/") # For zconf.h on WIN32

if(NOT USE_GLES2)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

if(APPLE)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/source/Irrlicht/MacOSX" "${CMAKE_CURRENT_SOURCE_DIR}/source/Irrlicht")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch i386")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch i386 -F/Library/Frameworks")
endif()

add_definitions(-DNDEBUG=1 -DIRRLICHT_EXPORTS=1 -DPNG_THREAD_UNSAFE_OK -DPNG_NO_MMX_CODE -DPNG_NO_MNG_FEATURES)
if(MSVC)
  add_definitions(/D_IRR_STATIC_LIB_)
  add_definitions(/D_CRT_SECURE_NO_WARNINGS) # Shut up about unsafe stuff
elseif(MINGW)
  add_definitions(-D_IRR_STATIC_LIB_)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS) # Shut up about unsafe stuff
else()
  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -Wall -pipe -O3  -fno-exceptions  -fstrict-aliasing -I/usr/X11R6/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -pipe -O3  -fno-exceptions  -fstrict-aliasing -I/usr/X11R6/include")
  if(CMAKE_COMPILER_IS_GNUCC)
	  set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fexpensive-optimizations")
  endif()
  if(CMAKE_COMPILER_IS_GNUCXX)
	  set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -fexpensive-optimizations")
  endif()
endif()

if(USE_GLES2)
    add_definitions(-D_IRR_COMPILE_WITH_OGLES2_ -DNO_IRR_COMPILE_WITH_SOFTWARE_ -DNO_IRR_COMPILE_WITH_BURNINGSVIDEO_ -DNO_IRR_COMPILE_WITH_OGLES1_ -DNO_IRR_COMPILE_WITH_OPENGL_ -DNO_IRR_USE_NON_SYSTEM_JPEG_LIB_ -DNO_IRR_USE_NON_SYSTEM_LIB_PNG_ -DNO_IRR_USE_NON_SYSTEM_ZLIB_)
endif()

# Xrandr
if(UNIX AND USE_XRANDR)
    add_definitions(-DNO_IRR_LINUX_X11_VIDMODE_)
    add_definitions(-D_IRR_LINUX_X11_RANDR_)
endif()

if(CYGWIN)
    add_definitions(-DNO_IRR_COMPILE_WITH_JOYSTICK_EVENTS_)
endif()


set(IRRLICHT_SOURCES
source/Irrlicht/CGeometryCreator.cpp
source/Irrlicht/C3DSMeshFileLoader.cpp
source/Irrlicht/CParticleSystemSceneNode.cpp
source/Irrlicht/CSceneNodeAnimatorDelete.cpp
source/Irrlicht/CGUICheckBox.cpp
source/Irrlicht/COGLES2MaterialRenderer.cpp
source/Irrlicht/CGUIButton.cpp
source/Irrlicht/CAnimatedMeshSceneNode.cpp
source/Irrlicht/CSkinnedMesh.cpp
source/Irrlicht/CSceneNodeAnimatorTexture.cpp
source/Irrlicht/CParticleBoxEmitter.cpp
source/Irrlicht/CTriangleBBSelector.cpp
source/Irrlicht/CGUIComboBox.cpp
source/Irrlicht/CGUIFileOpenDialog.cpp
source/Irrlicht/CLimitReadFile.cpp
source/Irrlicht/CParticleGravityAffector.cpp
source/Irrlicht/CGUIMeshViewer.cpp
source/Irrlicht/os.cpp
source/Irrlicht/CParticleCylinderEmitter.cpp
source/Irrlicht/COpenGLExtensionHandler.cpp
source/Irrlicht/CParticleRotationAffector.cpp
source/Irrlicht/COGLES2NormalMapRenderer.cpp
source/Irrlicht/CSceneNodeAnimatorFlyCircle.cpp
source/Irrlicht/COpenGLParallaxMapRenderer.cpp
source/Irrlicht/CGUIImage.cpp
source/Irrlicht/CVideoModeList.cpp
source/Irrlicht/COctreeSceneNode.cpp
source/Irrlicht/COGLES2Texture.cpp
source/Irrlicht/CSceneNodeAnimatorCollisionResponse.cpp
source/Irrlicht/CImageWriterBMP.cpp
source/Irrlicht/CMeshCache.cpp
source/Irrlicht/CIrrDeviceFB.cpp
source/Irrlicht/COpenGLDriver.cpp
source/Irrlicht/COctreeTriangleSelector.cpp
source/Irrlicht/CGUIMenu.cpp
source/Irrlicht/CMetaTriangleSelector.cpp
source/Irrlicht/CFileList.cpp
source/Irrlicht/CImageWriterJPG.cpp
source/Irrlicht/CImageLoaderJPG.cpp
source/Irrlicht/CParticlePointEmitter.cpp
source/Irrlicht/CTerrainTriangleSelector.cpp
source/Irrlicht/COGLES2FixedPipelineRenderer.cpp
source/Irrlicht/CAndroidAssetReader.cpp
source/Irrlicht/CIrrDeviceSDL.cpp
source/Irrlicht/CDefaultSceneNodeAnimatorFactory.cpp
source/Irrlicht/COGLES2Renderer2D.cpp
source/Irrlicht/CGUIStaticText.cpp
source/Irrlicht/CSceneNodeAnimatorCameraMaya.cpp
source/Irrlicht/CSceneManager.cpp
source/Irrlicht/CGUITreeView.cpp
source/Irrlicht/CDefaultGUIElementFactory.cpp
source/Irrlicht/CSceneCollisionManager.cpp
source/Irrlicht/CGUIInOutFader.cpp
source/Irrlicht/CSceneNodeAnimatorFlyStraight.cpp
source/Irrlicht/CColorConverter.cpp
source/Irrlicht/CLogger.cpp
source/Irrlicht/CSceneNodeAnimatorFollowSpline.cpp
source/Irrlicht/CGUIModalScreen.cpp
source/Irrlicht/CSkyDomeSceneNode.cpp
source/Irrlicht/CAndroidAssetFileArchive.cpp
source/Irrlicht/CGUITabControl.cpp
source/Irrlicht/CNullDriver.cpp
source/Irrlicht/CGUISpriteBank.cpp
source/Irrlicht/COGLES2Driver.cpp
source/Irrlicht/CLightSceneNode.cpp
source/Irrlicht/CBoneSceneNode.cpp
source/Irrlicht/CGUITable.cpp
source/Irrlicht/CParticleAttractionAffector.cpp
source/Irrlicht/CImage.cpp
source/Irrlicht/CB3DMeshFileLoader.cpp
source/Irrlicht/CParticleAnimatedMeshSceneNodeEmitter.cpp
source/Irrlicht/CGUIScrollBar.cpp
source/Irrlicht/CSkyBoxSceneNode.cpp
source/Irrlicht/CTerrainSceneNode.cpp
source/Irrlicht/COSOperator.cpp
source/Irrlicht/CImageLoaderPNG.cpp
source/Irrlicht/CMountPointReader.cpp
source/Irrlicht/CParticleSphereEmitter.cpp
source/Irrlicht/CDummyTransformationSceneNode.cpp
source/Irrlicht/CDefaultSceneNodeFactory.cpp
source/Irrlicht/CParticleFadeOutAffector.cpp
source/Irrlicht/CGUIEditBox.cpp
source/Irrlicht/COGLES2ParallaxMapRenderer.cpp
source/Irrlicht/CXMLReader.cpp
source/Irrlicht/COGLESDriver.cpp
source/Irrlicht/CIrrDeviceAndroid.cpp
source/Irrlicht/COGLESTexture.cpp
source/Irrlicht/CGUIWindow.cpp
source/Irrlicht/CParticleMeshEmitter.cpp
source/Irrlicht/CGUIEnvironment.cpp
source/Irrlicht/CTarReader.cpp
source/Irrlicht/CMeshManipulator.cpp
source/Irrlicht/CGUISpinBox.cpp
source/Irrlicht/CImageLoaderBMP.cpp
source/Irrlicht/CIrrDeviceWin32.cpp
source/Irrlicht/CTriangleSelector.cpp
source/Irrlicht/COpenGLSLMaterialRenderer.cpp
source/Irrlicht/CIrrDeviceStub.cpp
source/Irrlicht/irrXML.cpp
source/Irrlicht/COpenGLTexture.cpp
source/Irrlicht/CAttributes.cpp
source/Irrlicht/COGLES2ExtensionHandler.cpp
source/Irrlicht/COGLESExtensionHandler.cpp
source/Irrlicht/CImageWriterPNG.cpp
source/Irrlicht/CZBuffer.cpp
source/Irrlicht/CParticleRingEmitter.cpp
source/Irrlicht/CFileSystem.cpp
source/Irrlicht/COpenGLShaderMaterialRenderer.cpp
source/Irrlicht/CIrrDeviceLinux.cpp
source/Irrlicht/CCubeSceneNode.cpp
source/Irrlicht/CReadFile.cpp
source/Irrlicht/CZipReader.cpp
source/Irrlicht/CSphereSceneNode.cpp
source/Irrlicht/CGUIMessageBox.cpp
source/Irrlicht/CGUIContextMenu.cpp
source/Irrlicht/CGUIListBox.cpp
source/Irrlicht/CGUIToolBar.cpp
source/Irrlicht/CCameraSceneNode.cpp
source/Irrlicht/CXMeshFileLoader.cpp
source/Irrlicht/CParticleScaleAffector.cpp
source/Irrlicht/CBillboardSceneNode.cpp
source/Irrlicht/Irrlicht.cpp
source/Irrlicht/CEmptySceneNode.cpp
source/Irrlicht/CMemoryFile.cpp
source/Irrlicht/CFPSCounter.cpp
source/Irrlicht/CGUISkin.cpp
source/Irrlicht/CGUIColorSelectDialog.cpp
source/Irrlicht/CWaterSurfaceSceneNode.cpp
source/Irrlicht/CIrrDeviceConsole.cpp
source/Irrlicht/CTextSceneNode.cpp
source/Irrlicht/CSceneNodeAnimatorCameraFPS.cpp
source/Irrlicht/CGUIFont.cpp
source/Irrlicht/CWriteFile.cpp
source/Irrlicht/CMeshSceneNode.cpp
source/Irrlicht/CXMLWriter.cpp
source/Irrlicht/CGUIImageList.cpp
source/Irrlicht/CSceneNodeAnimatorRotation.cpp
)

if(APPLE)
    set(IRRLICHT_SOURCES
        ${IRRLICHT_SOURCES}
        source/Irrlicht/MacOSX/AppDelegate.mm
        source/Irrlicht/MacOSX/CIrrDeviceMacOSX.mm
        source/Irrlicht/MacOSX/OSXClipboard.mm)

    #list(APPEND CMAKE_C_SOURCE_FILE_EXTENSIONS mm)
    #set(CMAKE_CXX_SOURCE_FILE_EXTENSIONS cpp)

    set_source_files_properties(source/Irrlicht/MacOSX/AppDelegate.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -O3  -fno-rtti")
    set_source_files_properties(source/Irrlicht/MacOSX/AppDelegate.mm PROPERTIES LANGUAGE C)

    set_source_files_properties(source/Irrlicht/MacOSX/CIrrDeviceMacOSX.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -O3  -fno-rtti")
    set_source_files_properties(source/Irrlicht/MacOSX/CIrrDeviceMacOSX.mm PROPERTIES LANGUAGE C)

    set_source_files_properties(source/Irrlicht/MacOSX/OSXClipboard.mm PROPERTIES COMPILE_FLAGS "-x objective-c++ -O3 -fno-rtti")
    set_source_files_properties(source/Irrlicht/MacOSX/OSXClipboard.mm PROPERTIES LANGUAGE C)
endif()

add_library(stkirrlicht STATIC ${IRRLICHT_SOURCES})
target_link_libraries(stkirrlicht ${PNG_LIBRARY} ${JPEG_LIBRARY} ${ZLIB_LIBRARY})

if(WIN32)
    target_link_libraries(stkirrlicht imm32)
endif()
