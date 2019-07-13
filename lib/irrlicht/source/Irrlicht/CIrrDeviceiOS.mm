// Copyright (C) 2002-2008 Nikolaus Gebhardt
// Copyright (C) 2008 Redshift Software, Inc.
// Copyright (C) 2012 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#import "CIrrDeviceiOS.h"

#ifdef _IRR_COMPILE_WITH_IOS_DEVICE_

#include "IFileSystem.h"
#include "CTimer.h"
#include "CEAGLManager.h"
#include "COGLES2Driver.h"

#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>

/* Important information */

// The application state events and following methods: IrrlichtDevice::isWindowActive, IrrlichtDevice::isWindowFocused
// and IrrlichtDevice::isWindowMinimized works out of box only if you'll use built-in CIrrDelegateiOS,
// so _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_ must be enabled in this case. If you need a custom UIApplicationDelegate you must
// handle all application events yourself.

#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_

namespace irr
{
    class CIrrDeviceiOS;
}

/* CIrrDelegateiOS */

@interface CIrrDelegateiOS : NSObject<UIApplicationDelegate>

- (void)setDevice:(irr::CIrrDeviceiOS*)device;
- (bool)isActive;
- (bool)hasFocus;

@property (strong, nonatomic) UIWindow* window;

@end

@implementation CIrrDelegateiOS
{
    irr::CIrrDeviceiOS* Device;
    bool Active;
    bool Focus;
}

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)options
{
    Device = nil;
    Active = true;
    Focus = false;

    [self performSelectorOnMainThread:@selector(runIrrlicht) withObject:nil waitUntilDone:NO];

    return YES;
}

- (void)applicationWillTerminate:(UIApplication*)application
{
    if (Device != nil)
    {
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_WILL_TERMINATE;

        Device->postEventFromUser(ev);

        Device->closeDevice();
    }
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication*)application
{
    if (Device != nil)
    {
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_MEMORY_WARNING;

        Device->postEventFromUser(ev);
    }
}

- (void)applicationWillResignActive:(UIApplication*)application
{
    if (Device != nil)
    {
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_WILL_PAUSE;

        Device->postEventFromUser(ev);
    }

    Focus = false;
}

- (void)applicationDidEnterBackground:(UIApplication*)application
{
    if (Device != nil)
    {
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_DID_PAUSE;

        Device->postEventFromUser(ev);
    }

    Active = false;
}

- (void)applicationWillEnterForeground:(UIApplication*)application
{
    if (Device != nil)
    {
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_WILL_RESUME;

        Device->postEventFromUser(ev);
    }

    Active = true;
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
    if (Device != nil)
    {
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_DID_RESUME;

        Device->postEventFromUser(ev);
    }

    Focus = true;
}

- (void)runIrrlicht
{
    irrlicht_main();
}

- (void)setDevice:(irr::CIrrDeviceiOS*)device
{
    Device = device;
}

- (bool)isActive
{
    return Active;
}

- (bool)hasFocus
{
    return Focus;
}

@end

#endif

/* CIrrViewiOS */

@interface CIrrViewiOS : UIView

- (id)initWithFrame:(CGRect)frame forDevice:(irr::CIrrDeviceiOS*)device;

@end

@implementation CIrrViewiOS
{
    irr::CIrrDeviceiOS* Device;
    float Scale;
}

- (id)initWithFrame:(CGRect)frame forDevice:(irr::CIrrDeviceiOS*)device;
{
    self = [super initWithFrame:frame];

    if (self)
    {
        Device = device;
        Scale = ([self respondsToSelector:@selector(setContentScaleFactor:)]) ? [[UIScreen mainScreen] scale] : 1.f;
    }

    return self;
}

- (BOOL)isMultipleTouchEnabled
{
    return YES;
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_PRESSED_DOWN;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = (size_t)touch;

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x*Scale;
        ev.TouchInput.Y = touchPoint.y*Scale;

        Device->postEventFromUser(ev);
    }
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_MOVED;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = (size_t)touch;

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x*Scale;
        ev.TouchInput.Y = touchPoint.y*Scale;

        Device->postEventFromUser(ev);
    }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_LEFT_UP;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = (size_t)touch;

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x*Scale;
        ev.TouchInput.Y = touchPoint.y*Scale;

        Device->postEventFromUser(ev);
    }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_LEFT_UP;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = (size_t)touch;

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x*Scale;
        ev.TouchInput.Y = touchPoint.y*Scale;

        Device->postEventFromUser(ev);
    }
}

@end

/* CIrrViewEAGLiOS */

@interface CIrrViewEAGLiOS : CIrrViewiOS

@end

@implementation CIrrViewEAGLiOS

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

@end

namespace irr
{
    struct SIrrDeviceiOSDataStorage
    {
        SIrrDeviceiOSDataStorage() : Window(0), ViewController(0), View(0), MotionManager(0), ReferenceAttitude(0)
        {
            MotionManager = [[CMMotionManager alloc] init];
        }

        UIWindow* Window;
        UIViewController* ViewController;
        CIrrViewiOS* View;
        CMMotionManager* MotionManager;
        CMAttitude* ReferenceAttitude;
    };

    CIrrDeviceiOS::CIrrDeviceiOS(const SIrrlichtCreationParameters& params) : CIrrDeviceStub(params), ContextManager(0), DataStorage(0), Close(false)
    {
#ifdef _DEBUG
        setDebugName("CIrrDeviceiOS");
#endif

#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
        CIrrDelegateiOS* delegate = [UIApplication sharedApplication].delegate;
        [delegate setDevice:this];
#endif

        DataStorage = new SIrrDeviceiOSDataStorage();

        FileSystem->changeWorkingDirectoryTo([[[NSBundle mainBundle] resourcePath] UTF8String]);

        createWindow();
        createViewAndDriver();

        if (!VideoDriver)
            return;

        createGUIAndScene();
    }

    CIrrDeviceiOS::~CIrrDeviceiOS()
    {
        deactivateDeviceMotion();
        deactivateGyroscope();
        deactivateAccelerometer();

        delete static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
        CIrrDelegateiOS* delegate = [UIApplication sharedApplication].delegate;
        [delegate setDevice:nil];
#endif
        if (ContextManager)
            ContextManager->drop();
    }

    bool CIrrDeviceiOS::run()
    {
        if (!Close)
        {
            const CFTimeInterval timeInSeconds = 0.000002;

            s32 result = 0;

            do
            {
                result = CFRunLoopRunInMode(kCFRunLoopDefaultMode, timeInSeconds, TRUE);
            }
            while (result == kCFRunLoopRunHandledSource);

            os::Timer::tick();

            //! Update events

            SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
            CMMotionManager* motionManager = dataStorage->MotionManager;

            //! Accelerometer
            if (motionManager.isAccelerometerActive)
            {
                irr::SEvent ev;
                ev.EventType = irr::EET_ACCELEROMETER_EVENT;
                ev.AccelerometerEvent.X = motionManager.accelerometerData.acceleration.x;
                ev.AccelerometerEvent.Y = motionManager.accelerometerData.acceleration.y;
                ev.AccelerometerEvent.Z = motionManager.accelerometerData.acceleration.z;

                postEventFromUser(ev);
            }

            //! Gyroscope
            if (motionManager.isGyroActive)
            {
                irr::SEvent ev;
                ev.EventType = irr::EET_GYROSCOPE_EVENT;
                ev.GyroscopeEvent.X = motionManager.gyroData.rotationRate.x;
                ev.GyroscopeEvent.Y = motionManager.gyroData.rotationRate.y;
                ev.GyroscopeEvent.Z = motionManager.gyroData.rotationRate.z;

                postEventFromUser(ev);
            }

            //! Device Motion
            if (motionManager.isDeviceMotionActive)
            {
                CMAttitude* currentAttitude = motionManager.deviceMotion.attitude;
                CMAttitude* referenceAttitude = dataStorage->ReferenceAttitude;

                if (referenceAttitude != nil)
                    [currentAttitude multiplyByInverseOfAttitude: referenceAttitude];
                else
                    referenceAttitude = motionManager.deviceMotion.attitude;

                irr::SEvent ev;
                ev.EventType = irr::EET_DEVICE_MOTION_EVENT;
                ev.AccelerometerEvent.X = currentAttitude.roll;
                ev.AccelerometerEvent.Y = currentAttitude.pitch;
                ev.AccelerometerEvent.Z = currentAttitude.yaw;

                postEventFromUser(ev);
            }
        }

        return !Close;
    }

    void CIrrDeviceiOS::yield()
    {
        struct timespec ts = {0,0};
        nanosleep(&ts, NULL);
    }

    void CIrrDeviceiOS::sleep(u32 timeMs, bool pauseTimer=false)
    {
        bool wasStopped = Timer ? Timer->isStopped() : true;

        struct timespec ts;
        ts.tv_sec = (time_t) (timeMs / 1000);
        ts.tv_nsec = (long) (timeMs % 1000) * 1000000;

        if (pauseTimer && !wasStopped)
            Timer->stop();

        nanosleep(&ts, NULL);

        if (pauseTimer && !wasStopped)
            Timer->start();
    }

    void CIrrDeviceiOS::setWindowCaption(const wchar_t* text)
    {
    }

    bool CIrrDeviceiOS::isWindowActive() const
    {
#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
        CIrrDelegateiOS* delegate = [UIApplication sharedApplication].delegate;

        return [delegate isActive];
#else
        return false;
#endif
    }

    bool CIrrDeviceiOS::isWindowFocused() const
    {
#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
        CIrrDelegateiOS* delegate = [UIApplication sharedApplication].delegate;

        return [delegate hasFocus];
#else
        return false;
#endif
    }

    bool CIrrDeviceiOS::isWindowMinimized() const
    {
#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
        CIrrDelegateiOS* delegate = [UIApplication sharedApplication].delegate;

        return ![delegate isActive];
#else
        return false;
#endif
    }

    bool CIrrDeviceiOS::present(video::IImage* image, void * windowId, core::rect<s32>* src)
    {
        return false;
    }

    void CIrrDeviceiOS::closeDevice()
    {
        CFRunLoopStop(CFRunLoopGetMain());

        Close = true;
    }

    void CIrrDeviceiOS::setResizable(bool resize)
    {
    }

    void CIrrDeviceiOS::minimizeWindow()
    {
    }

    void CIrrDeviceiOS::maximizeWindow()
    {
    }

    void CIrrDeviceiOS::restoreWindow()
    {
    }

    bool CIrrDeviceiOS::getWindowPosition(int* x, int* y)
    {
        *x = 0;
        *y = 0;
        return true;
    }

    bool CIrrDeviceiOS::activateAccelerometer(float updateInterval)
    {
        bool status = false;

        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
        CMMotionManager* motionManager = dataStorage->MotionManager;

        if (motionManager.isAccelerometerAvailable)
        {
            if (!motionManager.isAccelerometerActive)
            {
                motionManager.accelerometerUpdateInterval = updateInterval;
                [motionManager startAccelerometerUpdates];
            }

            status = true;
        }

        return status;
    }

    bool CIrrDeviceiOS::deactivateAccelerometer()
    {
        bool status = false;

        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
        CMMotionManager* motionManager = dataStorage->MotionManager;

        if (motionManager.isAccelerometerAvailable)
        {
            if (motionManager.isAccelerometerActive)
                [motionManager stopAccelerometerUpdates];

            status = true;
        }

        return status;
    }

    bool CIrrDeviceiOS::isAccelerometerActive()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        return (dataStorage->MotionManager.isAccelerometerActive);
    }

    bool CIrrDeviceiOS::isAccelerometerAvailable()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        return (dataStorage->MotionManager.isAccelerometerAvailable);
    }

    bool CIrrDeviceiOS::activateGyroscope(float updateInterval)
    {
        bool status = false;

        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
        CMMotionManager* motionManager = dataStorage->MotionManager;

        if (motionManager.isGyroAvailable)
        {
            if (!motionManager.isGyroActive)
            {
                motionManager.gyroUpdateInterval = updateInterval;
                [motionManager startGyroUpdates];
            }

            status = true;
        }

        return status;
    }

    bool CIrrDeviceiOS::deactivateGyroscope()
    {
        bool status = false;

        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
        CMMotionManager* motionManager = dataStorage->MotionManager;

        if (motionManager.isGyroAvailable)
        {
            if (motionManager.isGyroActive)
                [motionManager stopGyroUpdates];

            status = true;
        }

        return status;
    }

    bool CIrrDeviceiOS::isGyroscopeActive()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        return (dataStorage->MotionManager.isGyroActive);
    }

    bool CIrrDeviceiOS::isGyroscopeAvailable()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        return (dataStorage->MotionManager.isGyroAvailable);
    }

    bool CIrrDeviceiOS::activateDeviceMotion(float updateInterval)
    {
        bool status = false;

        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
        CMMotionManager* motionManager = dataStorage->MotionManager;

        if (motionManager.isDeviceMotionAvailable)
        {
            if (!motionManager.isDeviceMotionActive)
            {
                dataStorage->ReferenceAttitude = nil;

                motionManager.deviceMotionUpdateInterval = updateInterval;
                [motionManager startDeviceMotionUpdates];
            }

            status = true;
        }

        return status;
    }

    bool CIrrDeviceiOS::deactivateDeviceMotion()
    {
        bool status = false;

        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);
        CMMotionManager* motionManager = dataStorage->MotionManager;

        if (motionManager.isDeviceMotionAvailable)
        {
            if (motionManager.isDeviceMotionActive)
            {
                [motionManager stopDeviceMotionUpdates];

                dataStorage->ReferenceAttitude = nil;
            }

            status = true;
        }

        return status;
    }

    bool CIrrDeviceiOS::isDeviceMotionActive()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        return (dataStorage->MotionManager.isDeviceMotionActive);
    }

    bool CIrrDeviceiOS::isDeviceMotionAvailable()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        return (dataStorage->MotionManager.isDeviceMotionAvailable);
    }

    E_DEVICE_TYPE CIrrDeviceiOS::getType() const
    {
        return EIDT_IOS;
    }

    void CIrrDeviceiOS::createWindow()
    {
        if (CreationParams.DriverType != video::EDT_NULL)
        {
            SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

            UIView* externalView = (__bridge UIView*)CreationParams.WindowId;

            if (externalView == nil)
            {
                dataStorage->Window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
                dataStorage->ViewController = [[UIViewController alloc] init];
                dataStorage->Window.rootViewController = dataStorage->ViewController;

                [dataStorage->Window makeKeyAndVisible];
            }
            else
            {
                dataStorage->Window = externalView.window;

                UIResponder* currentResponder = externalView.nextResponder;

                do
                {
                    if ([currentResponder isKindOfClass:[UIViewController class]])
                    {
                        dataStorage->ViewController = (UIViewController*)currentResponder;

                        currentResponder = nil;
                    }
                    else if ([currentResponder isKindOfClass:[UIView class]])
                    {
                        currentResponder = currentResponder.nextResponder;
                    }
                    else
                    {
                        currentResponder = nil;

                        // Could not find view controller.
                        _IRR_DEBUG_BREAK_IF(true);
                    }
                }
                while (currentResponder != nil);
            }
        }
    }

    void CIrrDeviceiOS::createViewAndDriver()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        video::SExposedVideoData data;
        data.OpenGLiOS.Window = (__bridge void*)dataStorage->Window;
        data.OpenGLiOS.ViewController = (__bridge void*)dataStorage->ViewController;

        UIView* externalView = (__bridge UIView*)CreationParams.WindowId;

        CGRect resolution = (externalView == nil) ? [[UIScreen mainScreen] bounds] : externalView.bounds;

        switch (CreationParams.DriverType)
        {
            case video::EDT_OGLES2:
#ifdef _IRR_COMPILE_WITH_OGLES2_
                {
                    CIrrViewEAGLiOS* view = [[CIrrViewEAGLiOS alloc] initWithFrame:resolution forDevice:this];
                    CreationParams.WindowSize = core::dimension2d<u32>(view.frame.size.width, view.frame.size.height);

                    dataStorage->View = view;
                    data.OpenGLiOS.View = (__bridge void*)view;

                    ContextManager = new irr::video::CEAGLManager();
                    ContextManager->initialize(CreationParams, data);

                    VideoDriver = new video::COGLES2Driver(CreationParams, FileSystem, this, ContextManager);

                    if (!VideoDriver)
                        os::Printer::log("Could not create OpenGL ES 2.x driver.", ELL_ERROR);
                }
#else
                os::Printer::log("No OpenGL ES 2.x support compiled in.", ELL_ERROR);
#endif
                break;

            case video::EDT_SOFTWARE:
            case video::EDT_BURNINGSVIDEO:
            case video::EDT_DIRECT3D9:
            case video::EDT_OPENGL:
                os::Printer::log("This driver is not available in iOS. Try OpenGL ES.", ELL_ERROR);
                break;

            case video::EDT_NULL:
                VideoDriver = video::createNullDriver(FileSystem, CreationParams.WindowSize);
                break;

            default:
                os::Printer::log("Unable to create video driver of unknown type.", ELL_ERROR);
                break;
        }

        if (externalView == nil)
            dataStorage->ViewController.view = dataStorage->View;
        else
            [externalView addSubview:dataStorage->View];
    }
}

#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
int main(int argc, char** argv)
{
    int result = UIApplicationMain(argc, argv, 0, NSStringFromClass([CIrrDelegateiOS class]));

    return result;
}
#endif

#endif
