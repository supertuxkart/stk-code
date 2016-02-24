/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

#include <irrlicht.h>

/*
In the Irrlicht Engine, everything can be found in the namespace 'irr'. So if
you want to use a class of the engine, you have to write irr:: before the name
of the class. For example to use the IrrlichtDevice write: irr::IrrlichtDevice.
To get rid of the irr:: in front of the name of every class, we tell the
compiler that we use that namespace from now on, and we will not have to write
irr:: anymore.
*/
using namespace irr;

/*
There are 5 sub namespaces in the Irrlicht Engine. Take a look at them, you can
read a detailed description of them in the documentation by clicking on the top
menu item 'Namespace List' or by using this link:
http://irrlicht.sourceforge.net/docu/namespaces.html
Like the irr namespace, we do not want these 5 sub namespaces now, to keep this
example simple. Hence, we tell the compiler again that we do not want always to
write their names.
*/
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main_2() {

    /*
    The most important function of the engine is the createDevice()
    function. The IrrlichtDevice is created by it, which is the root
    object for doing anything with the engine. createDevice() has 7
    parameters:

    - deviceType: Type of the device. This can currently be the Null-device,
       one of the two software renderers, D3D8, D3D9, or OpenGL. In this
       example we use EDT_SOFTWARE, but to try out, you might want to
       change it to EDT_BURNINGSVIDEO, EDT_NULL, EDT_DIRECT3D8,
       EDT_DIRECT3D9, or EDT_OPENGL.

    - windowSize: Size of the Window or screen in FullScreenMode to be
       created. In this example we use 640x480.

    - bits: Amount of color bits per pixel. This should be 16 or 32. The
       parameter is often ignored when running in windowed mode.

    - fullscreen: Specifies if we want the device to run in fullscreen mode
       or not.

    - stencilbuffer: Specifies if we want to use the stencil buffer (for
       drawing shadows).

    - vsync: Specifies if we want to have vsync enabled, this is only useful
       in fullscreen mode.

    - eventReceiver: An object to receive events. We do not want to use this
       parameter here, and set it to 0.

    Always check the return value to cope with unsupported drivers,
    dimensions, etc.
    */
    IrrlichtDevice *device =
        createDevice( video::EDT_OGLES1, dimension2d<u32>(640, 480), 16,
            false, false, false, 0);

    if (!device)
        return ;

    /*
    Set the caption of the window to some nice text. Note that there is an
    'L' in front of the string. The Irrlicht Engine uses wide character
    strings when displaying text.
    */
    device->setWindowCaption(L"Hello World! - Irrlicht Engine Demo");

    /*
    Get a pointer to the VideoDriver, the SceneManager and the graphical
    user interface environment, so that we do not always have to write
    device->getVideoDriver(), device->getSceneManager(), or
    device->getGUIEnvironment().
    */
    IVideoDriver* driver = device->getVideoDriver();
    ISceneManager* smgr = device->getSceneManager();
    IGUIEnvironment* guienv = device->getGUIEnvironment();

    /*
    We add a hello world label to the window, using the GUI environment.
    The text is placed at the position (10,10) as top left corner and
    (260,22) as lower right corner.
    */
    guienv->addStaticText(L"Hello World! This is the Irrlicht Software renderer!",
        rect<s32>(10,10,260,22), true);

    /*
    To look at the mesh, we place a camera into 3d space at the position
    (0, 30, -40). The camera looks from there to (0,5,0), which is
    approximately the place where our md2 model is.
    */
    smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));

    /*
    Ok, now we have set up the scene, lets draw everything: We run the
    device in a while() loop, until the device does not want to run any
    more. This would be when the user closes the window or presses ALT+F4
    (or whatever keycode closes a window).
    */
    while(device->run())
    {
        /*
        Anything can be drawn between a beginScene() and an endScene()
        call. The beginScene() call clears the screen with a color and
        the depth buffer, if desired. Then we let the Scene Manager and
        the GUI Environment draw their content. With the endScene()
        call everything is presented on the screen.
        */
        driver->beginScene(true, true, SColor(255,100,101,140));

        smgr->drawAll();
        guienv->drawAll();

        driver->endScene();
    }

    /*
    After we are done with the render loop, we have to delete the Irrlicht
    Device created before with createDevice(). In the Irrlicht Engine, you
    have to delete all objects you created with a method or function which
    starts with 'create'. The object is simply deleted by calling ->drop().
    See the documentation at irr::IReferenceCounted::drop() for more
    information.
    */
    device->drop();
}
//END_INCLUDE(all)
