// Copyright (C) 2002-2007 Nikolaus Gebhardt
// Copyright (C) 2007-2011 Christian Stehno
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CIrrDeviceAndroid.h"

#ifdef _IRR_COMPILE_WITH_ANDROID_DEVICE_

#include <assert.h>
#include "os.h"
#include "CFileSystem.h"
#include "CAndroidAssetFileArchive.h"

namespace irr	
{
	namespace video
	{
		IVideoDriver* createOGLES1Driver(const SIrrlichtCreationParameters& params,	
			video::SExposedVideoData& data, io::IFileSystem* io);

		IVideoDriver* createOGLES2Driver(const SIrrlichtCreationParameters& params,
			video::SExposedVideoData& data, io::IFileSystem* io);
	}	
}

namespace irr
{

android_app* CIrrDeviceAndroid::Android = NULL;
#include <android/log.h>

//! constructor
CIrrDeviceAndroid::CIrrDeviceAndroid(const SIrrlichtCreationParameters& param)
	: CIrrDeviceStub(param),
	Animating(false),
	IsReady(false),
	IsClosing(false),
	DeviceWidth(param.WindowSize.Width),
	DeviceHeight(param.WindowSize.Height),
	MouseX(0),
	MouseY(0)
{
	int ident;
	int events;
	struct android_poll_source* source;
	
	#ifdef _DEBUG
	setDebugName("CIrrDeviceAndroid");
	#endif
	
	// Get the interface to the native Android activity.
	Android = (android_app *)(param.PrivateData);
	
	// Set the private data so we can use it in any static callbacks.
	Android->userData = this;
		
	// Set the default command handler. This is a callback function
	// that the Android OS invokes to send the native activity
	// messages.
	Android->onAppCmd = handleAndroidCommand;
	
	// Create a sensor manager to recieve touch screen events from the
	// java acivity.
	SensorManager = ASensorManager_getInstance();
	SensorEventQueue = ASensorManager_createEventQueue(
						SensorManager, Android->looper,
						LOOPER_ID_USER, NULL, NULL);
	Android->onInputEvent = handleInput;
                          	
	// Need to find a better way of doing this... but poll until the
	// Android activity has been created and a window is open. The device
	// cannot be created until the main window has been created by the
	// Android OS.
	os::Printer::log("Waiting for Android activity window to be created.", ELL_DEBUG);

	do	
	{
		while( (ident = ALooper_pollAll(	0, NULL, &events,(void**)&source)) >= 0 )
		{
			// Process this event.
			if( source != NULL )
			{
				source->process( Android, source );
			}
		}	
	}
	while( IsReady == false );
	
	assert( Android->window );

	// Create cursor control
	CursorControl = new CCursorControl(this);

	// Create the driver.
	createDriver();
		
	if (VideoDriver)	
		createGUIAndScene();
		
	io::CAndroidAssetFileArchive *assets = io::createAndroidAssetFileArchive(false, false);
	assets->addDirectory("media");
	FileSystem->addFileArchive(assets);
	// TODO
	//
	// if engine->app->savedState is not NULL then use postEventFromUser() 
	// with a custom android event so the user can use their own event 
	// receiver in order to load the apps previous state.
	// The message should have a pointer to be filled and a size variable
	// of how much data has been saved. Android free's this data later so
	// there's no need to free it manually.
	
	Animating = true;
}


CIrrDeviceAndroid::~CIrrDeviceAndroid(void)
{
}

void CIrrDeviceAndroid::createDriver( void )
{
	video::SExposedVideoData data;
		
	// Create the driver.
	switch(CreationParams.DriverType)
	{
	case video::EDT_OGLES1:
		#ifdef _IRR_COMPILE_WITH_OGLES1_		
		VideoDriver = video::createOGLES1Driver(CreationParams, data, FileSystem);
		#else
		os::Printer::log("No OpenGL ES 1.0 support compiled in.", ELL_ERROR);
		#endif
		break;
		
	case video::EDT_OGLES2:
		#ifdef _IRR_COMPILE_WITH_OGLES2_
		VideoDriver = video::createOGLES2Driver(CreationParams, data, FileSystem);
		#else
		os::Printer::log("No OpenGL ES 2.0 support compiled in.", ELL_ERROR);
		#endif
		break;		

	case video::EDT_NULL:
		VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
		break;

	default:
		os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
		break;
	}	
}

bool CIrrDeviceAndroid::run( void )
{
	bool close = false;
	
	// Read all pending events.
	int ident;
	int events;
	struct android_poll_source* source;

	// Check if Android is trying to shut us down.
	if( IsClosing == true )
		return( false );
		
	// If not animating, we will block forever waiting for events.
	// If animating, we loop until all events are read, then continue
	// to draw the next frame of animation.
	while( ( ident = ALooper_pollAll(	( Animating || IsClosing ) ? 0 : -1, 
										NULL, &events,
										(void**)&source)) >= 0 )
    {
		// Process this event.
		if( source != NULL )
		{
			source->process( Android, source );
		}
#if 0
         // If a sensor has data, process it now.
         if (ident == LOOPER_ID_USER) {
             if (engine.accelerometerSensor != NULL) {
                 ASensorEvent event;
                 while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                         &event, 1) > 0) {
                    DEBUG_INFO1("accelerometer: x=%f y=%f z=%f",
                            event.acceleration.x, event.acceleration.y,
                             event.acceleration.z);
                 }
             }
         }
#endif
		// Check if we are exiting.
		if( Android->destroyRequested != 0 )
		{                  
			close = true;
		}
	}
    	
	os::Timer::tick();
	return( !close );
}

void CIrrDeviceAndroid::yield( void )
{
}

void CIrrDeviceAndroid::sleep( u32 timeMs, bool pauseTimer )
{
}

void CIrrDeviceAndroid::setWindowCaption(const wchar_t* text)
{
}

bool CIrrDeviceAndroid::present(video::IImage* surface, void* windowId, core::rect<s32>* srcClip)
{
	return( true );
}

bool CIrrDeviceAndroid::isWindowActive( void ) const
{
	return( true );
}

bool CIrrDeviceAndroid::isWindowFocused( void ) const
{
	return( false );
}

bool CIrrDeviceAndroid::isWindowMinimized( void ) const
{
	return( false );
}

void CIrrDeviceAndroid::closeDevice( void )
{
}

void CIrrDeviceAndroid::setResizable( bool resize )
{
}

void CIrrDeviceAndroid::minimizeWindow( void )
{
}

void CIrrDeviceAndroid::maximizeWindow( void )
{
}

void CIrrDeviceAndroid::restoreWindow( void )
{
}

E_DEVICE_TYPE CIrrDeviceAndroid::getType( void ) const
{
	return( EIDT_ANDROID );
}
		
///////////////////////////////
///////////////////////////////

void CIrrDeviceAndroid::handleAndroidCommand( struct android_app* app, s32 cmd )
{
    CIrrDeviceAndroid *deviceAndroid = (CIrrDeviceAndroid *)app->userData;
    switch (cmd)
    {
        case APP_CMD_SAVE_STATE:
			os::Printer::log("Android command APP_CMD_SAVE_STATE", ELL_DEBUG);        
			
			// TODO
			//
			// use postEventFromUser() with a custom android event so the user can
			// use their own event receiver in order to save the apps state.
			// The message should have a pointer to be filled and a size variable
			// of ho emuch data has been saved.
#if 0
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
#endif
            break;
        case APP_CMD_INIT_WINDOW:
			os::Printer::log("Android command APP_CMD_INIT_WINDOW", ELL_DEBUG);
            deviceAndroid->IsReady = true;
            break;
        case APP_CMD_TERM_WINDOW:
			os::Printer::log("Android command APP_CMD_TERM_WINDOW", ELL_DEBUG);			
            break;
        case APP_CMD_GAINED_FOCUS:
			os::Printer::log("Android command APP_CMD_GAINED_FOCUS", ELL_DEBUG);        
#if 0
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
#endif
			deviceAndroid->Animating = true;
            break;
        case APP_CMD_LOST_FOCUS:
			os::Printer::log("Android command APP_CMD_LOST_FOCUS", ELL_DEBUG);
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
#if 0
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
#endif
            // Also stop animating.
            deviceAndroid->Animating = false;

            break;
		case APP_CMD_DESTROY:
			// The application is being destroyed. We must close the native
			// acitivity code and clean up otherwise the acitivity will stay
			// active.
			os::Printer::log("Android command APP_CMD_DESTROY", ELL_DEBUG);
			deviceAndroid->IsClosing = true;		
			break;
			
		case APP_CMD_PAUSE:
			os::Printer::log("Android command APP_CMD_PAUSE", ELL_DEBUG);
			break;
		
		case APP_CMD_STOP:
			os::Printer::log("Android command APP_CMD_STOP", ELL_DEBUG);
			break;
					
		case APP_CMD_RESUME:
			os::Printer::log("Android command APP_CMD_RESUME", ELL_DEBUG);
			break;		
		
		default:
			os::Printer::log("Unhandled android command",
			                 core::stringc(cmd).c_str(), ELL_WARNING );
						
    }
}

s32 CIrrDeviceAndroid::handleInput( struct android_app* app, AInputEvent* androidEvent )
{
	CIrrDeviceAndroid *deviceAndroid = (CIrrDeviceAndroid *)app->userData;
	int32_t eventAction;
	int pointerCount = 0;
	SEvent irrEvent;
	int i = 0;
	
	if( AInputEvent_getType( androidEvent ) == AINPUT_EVENT_TYPE_MOTION )
	{
		// Get the number of pointers.
		pointerCount = AMotionEvent_getPointerCount( androidEvent );

		// Get the actual input event type.
		eventAction = AMotionEvent_getAction( androidEvent );

		switch( eventAction )
		{
			case AMOTION_EVENT_ACTION_DOWN:
				if( pointerCount == 1 )
				{
					irrEvent.EventType = EET_MOUSE_INPUT_EVENT;
					irrEvent.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
				}
				break;

			case AMOTION_EVENT_ACTION_MOVE:
				if( pointerCount == 1 )
				{
					irrEvent.EventType = EET_MOUSE_INPUT_EVENT;
					irrEvent.MouseInput.Event = EMIE_MOUSE_MOVED;
				}
				else
				{
					irrEvent.EventType = EET_MULTI_TOUCH_EVENT;
					irrEvent.MultiTouchInput.Event = EMTIE_MOVED;					
				}	

				deviceAndroid->postEventFromUser(irrEvent);			
				break;

			case AMOTION_EVENT_ACTION_UP:
				if( pointerCount == 1 )
				{
					irrEvent.EventType = EET_MOUSE_INPUT_EVENT;
					irrEvent.MouseInput.Event = EMIE_LMOUSE_LEFT_UP;
				}						
				break;
			default:
				os::Printer::log("Unhandled motion event",
								 core::stringc(eventAction).c_str(),
								 ELL_WARNING );
		}
				
		if( pointerCount == 1 )
		{
			// Fill in the details for a one touch event.
			deviceAndroid->MouseX = irrEvent.MouseInput.X = AMotionEvent_getX(androidEvent, 0);
			deviceAndroid->MouseY = irrEvent.MouseInput.Y = AMotionEvent_getY(androidEvent, 0);
			irrEvent.MouseInput.ButtonStates = 0;
		}
		else if( pointerCount > 1 )
		{
			// Fill in the details for a multi touch event.
			for( i=0 ; i<pointerCount ; i++ )
			{
				irrEvent.MultiTouchInput.Touched[i] = 1;
				irrEvent.MultiTouchInput.PrevX[i] = irrEvent.MultiTouchInput.X[i];
				irrEvent.MultiTouchInput.PrevY[i] = irrEvent.MultiTouchInput.Y[i];
				irrEvent.MultiTouchInput.X[i] = AMotionEvent_getX(androidEvent, i);
				irrEvent.MultiTouchInput.Y[i] = AMotionEvent_getY(androidEvent, i);
			}
			
			// Reset the data for the rest of the pointers that aren't being used.
			for( ;i<NUMBER_OF_MULTI_TOUCHES ; i++ )
			{
				irrEvent.MultiTouchInput.Touched[i] = 0;
				irrEvent.MultiTouchInput.PrevX[i] = 0;
				irrEvent.MultiTouchInput.PrevY[i] = 0;
				irrEvent.MultiTouchInput.X[i] = 0;
				irrEvent.MultiTouchInput.Y[i] = 0;
			}			
		}
		else
		{
			// Shouldn't ever get here, but just in case...
			os::Printer::log("We had a input event but no pointers were active!", ELL_DEBUG);
			return( 0 );
		}
		
		deviceAndroid->postEventFromUser(irrEvent);
		
		return( 1 );
	}
	return( 0 );
}

} // end namespace irr

#endif


