// Copyright (C) 2002-2008 Nikolaus Gebhardt
// Copyright (C) 2008 Redshift Software, Inc.
// Copyright (C) 2012 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#import "CIrrDeviceiOS.h"

#ifdef _IRR_COMPILE_WITH_IOS_DEVICE_

#include "IFileSystem.h"
#include "CTimer.h"
#include "COGLES2Driver.h"
#include "MobileCursorControl.h"

#import <CoreMotion/CoreMotion.h>
#import <GLKit/GLKView.h>
#import <sys/utsname.h>

extern void getConfigForDevice(const char* dev);

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

@interface HideStatusBarView : UIViewController
-(BOOL)prefersStatusBarHidden;
-(BOOL)prefersHomeIndicatorAutoHidden;
-(void)viewDidAppear:(BOOL)animated;
-(void)viewDidLoad;
-(void)dealloc;
@end

@implementation HideStatusBarView
{
    irr::CIrrDeviceiOS* Device;
}

-(void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
    [super dealloc];
}

- (id)init: (irr::CIrrDeviceiOS*)device
{
    self = [super init];
    Device = device;
    return self;
}

-(BOOL)prefersStatusBarHidden
{
    return YES;
}

-(BOOL)prefersHomeIndicatorAutoHidden
{
    return YES;
}

- (void)orientationChanged:(NSNotification*)note
{
    UIDevice* dev = [UIDevice currentDevice];
    if (dev == nil)
        return;

    UIDeviceOrientation orientation = [dev orientation];
    switch(orientation)
    {
        case UIDeviceOrientationLandscapeLeft:
            Device->setUpsideDown(true);
            break;
        case UIDeviceOrientationLandscapeRight:
            Device->setUpsideDown(false);
            break;
        default:
            break;
    };
}

-(void)viewDidLoad
{
    [super viewDidLoad];
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    [[NSNotificationCenter defaultCenter]
     addObserver:self selector:@selector(orientationChanged:)
     name:UIDeviceOrientationDidChangeNotification
     object:nil];

    // On first lanuch orientationChanged returns UIDeviceOrientationUnknown,
    // so we get the first oritentation from statusBarOrientation
    UIInterfaceOrientation orientation = [UIApplication sharedApplication].statusBarOrientation;
    // Those values are inverted with UIDeviceOrientation enum for landscape
    if (orientation == UIInterfaceOrientationLandscapeLeft)
        Device->setUpsideDown(false);
    else if (orientation == UIInterfaceOrientationLandscapeRight)
        Device->setUpsideDown(true);
}

-(void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear : animated];
    if (@available(iOS 11.0, *))
    {
        Device->setPaddings(self.view.safeAreaInsets.top,
            self.view.safeAreaInsets.bottom,
            self.view.safeAreaInsets.left,
            self.view.safeAreaInsets.right);
    }
}

@end

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

    [self performSelectorOnMainThread:@selector(runSTK) withObject:nil waitUntilDone:NO];

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
        Device->clearAllTouchIds();
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
        Device->clearAllTouchIds();
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
        Device->clearAllTouchIds();
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
        Device->clearAllTouchIds();
        irr::SEvent ev;
        ev.EventType = irr::EET_APPLICATION_EVENT;
        ev.ApplicationEvent.EventType = irr::EAET_DID_RESUME;

        Device->postEventFromUser(ev);
    }

    Focus = true;
}

- (void)runSTK
{
    override_default_params_for_mobile();
    struct utsname system_info;
    uname(&system_info);
    NSString* model = [NSString stringWithCString:system_info.machine
        encoding:NSUTF8StringEncoding];
    getConfigForDevice([model UTF8String]);
    ios_main(0, {});
    // App store may not like this
    exit(0);
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

@interface CIrrViewiOS : GLKView

- (id)initWithFrame:(CGRect)frame forDevice:(irr::CIrrDeviceiOS*)device forContext:(EAGLContext*)eagl_context;
- (void)setDevice:(irr::CIrrDeviceiOS*)device;

@end

@implementation CIrrViewiOS
{
    irr::CIrrDeviceiOS* Device;
}

- (id)initWithFrame:(CGRect)frame forDevice:(irr::CIrrDeviceiOS*)device forContext:(EAGLContext*)eagl_context
{
    self = [super initWithFrame:(frame) context:(eagl_context)];
    if (self)
    {
        self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
        self.multipleTouchEnabled = YES;
        Device = device;
    }

    return self;
}

- (void)setDevice:(irr::CIrrDeviceiOS*)device
{
    Device = device;
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event
{
    if (Device == nil)
        return;

    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_PRESSED_DOWN;

    irr::core::position2d<irr::s32> mouse_pos = irr::core::position2d<irr::s32>(0, 0);
    bool simulate_mouse = false;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = Device->getTouchId(touch);

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x * self.contentScaleFactor;
        ev.TouchInput.Y = touchPoint.y * self.contentScaleFactor;

        Device->postEventFromUser(ev);
        if (ev.TouchInput.ID == 0)
        {
            simulate_mouse = true;
            mouse_pos.X = ev.TouchInput.X;
            mouse_pos.Y = ev.TouchInput.Y;
        }
    }
    if (simulate_mouse)
        Device->simulateMouse(ev, mouse_pos);
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event
{
    if (Device == nil)
        return;

    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_MOVED;

    irr::core::position2d<irr::s32> mouse_pos = irr::core::position2d<irr::s32>(0, 0);
    bool simulate_mouse = false;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = Device->getTouchId(touch);

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x * self.contentScaleFactor;
        ev.TouchInput.Y = touchPoint.y * self.contentScaleFactor;

        Device->postEventFromUser(ev);
        if (ev.TouchInput.ID == 0)
        {
            simulate_mouse = true;
            mouse_pos.X = ev.TouchInput.X;
            mouse_pos.Y = ev.TouchInput.Y;
        }
    }
    if (simulate_mouse)
        Device->simulateMouse(ev, mouse_pos);
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event
{
    if (Device == nil)
        return;

    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_LEFT_UP;

    irr::core::position2d<irr::s32> mouse_pos = irr::core::position2d<irr::s32>(0, 0);
    bool simulate_mouse = false;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = Device->getTouchId(touch);
        Device->removeTouchId(touch);

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x * self.contentScaleFactor;
        ev.TouchInput.Y = touchPoint.y * self.contentScaleFactor;

        Device->postEventFromUser(ev);
        if (ev.TouchInput.ID == 0)
        {
            simulate_mouse = true;
            mouse_pos.X = ev.TouchInput.X;
            mouse_pos.Y = ev.TouchInput.Y;
        }
    }
    if (simulate_mouse)
        Device->simulateMouse(ev, mouse_pos);
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event
{
    if (Device == nil)
        return;

    irr::SEvent ev;
    ev.EventType = irr::EET_TOUCH_INPUT_EVENT;
    ev.TouchInput.Event = irr::ETIE_LEFT_UP;

    irr::core::position2d<irr::s32> mouse_pos = irr::core::position2d<irr::s32>(0, 0);
    bool simulate_mouse = false;

    for (UITouch* touch in touches)
    {
        ev.TouchInput.ID = Device->getTouchId(touch);
        Device->removeTouchId(touch);

        CGPoint touchPoint = [touch locationInView:self];

        ev.TouchInput.X = touchPoint.x * self.contentScaleFactor;
        ev.TouchInput.Y = touchPoint.y * self.contentScaleFactor;

        Device->postEventFromUser(ev);
        if (ev.TouchInput.ID == 0)
        {
            simulate_mouse = true;
            mouse_pos.X = ev.TouchInput.X;
            mouse_pos.Y = ev.TouchInput.Y;
        }
    }
    if (simulate_mouse)
        Device->simulateMouse(ev, mouse_pos);
}

@end



namespace irr
{
    struct SIrrDeviceiOSDataStorage
    {
        SIrrDeviceiOSDataStorage() : Window(0), ViewController(0), View(0), MotionManager(0), ReferenceAttitude(0)
        {
            MotionManager = [[CMMotionManager alloc] init];
            m_eagl_context = 0;
        }
        ~SIrrDeviceiOSDataStorage()
        {
            [Window release];
            [ViewController release];
            if (View != nil)
                [View setDevice:nil];
            [View release];
            [MotionManager release];
            [EAGLContext setCurrentContext:0];
            [m_eagl_context release];
        }
        UIWindow* Window;
        UIViewController* ViewController;
        CIrrViewiOS* View;
        CMMotionManager* MotionManager;
        CMAttitude* ReferenceAttitude;
        EAGLContext* m_eagl_context;
    };

    CIrrDeviceiOS::CIrrDeviceiOS(const SIrrlichtCreationParameters& params)
                 : CIrrDeviceStub(params), DataStorage(0), Close(false), m_upside_down(false)
    {
        m_top_padding = 0.0f;
        m_bottom_padding = 0.0f;
        m_left_padding = 0.0f;
        m_right_padding = 0.0f;
        m_native_scale = 1.0f;
#ifdef _DEBUG
        setDebugName("CIrrDeviceiOS");
#endif

#ifdef _IRR_COMPILE_WITH_IOS_BUILTIN_MAIN_
        CIrrDelegateiOS* delegate = [UIApplication sharedApplication].delegate;
        [delegate setDevice:this];
#endif

        DataStorage = new SIrrDeviceiOSDataStorage();

        FileSystem->changeWorkingDirectoryTo([[[NSBundle mainBundle] resourcePath] UTF8String]);

        if (VideoModeList.getVideoModeCount() == 0)
        {
            // Add current screen size
            CGRect screen_bounds = [[UIScreen mainScreen] nativeBounds];
            // nativeBounds is the size in a portrait-up orientation, so reverse width and height
            core::dimension2du screen_size =
            {
                (u32)screen_bounds.size.height,
                (u32)screen_bounds.size.width
            };
            VideoModeList.addMode(screen_size, 32);
            VideoModeList.setDesktop(32, screen_size);

        }
        createWindow();
        createViewAndDriver();

        if (!VideoDriver)
            return;

        createGUIAndScene();
        CursorControl = new gui::MobileCursorControl();
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
                ev.AccelerometerEvent.X = motionManager.accelerometerData.acceleration.x * 9.81;
                ev.AccelerometerEvent.Y = motionManager.accelerometerData.acceleration.y * 9.81;
                ev.AccelerometerEvent.Z = motionManager.accelerometerData.acceleration.z * 9.81;
                if (m_upside_down)
                    ev.AccelerometerEvent.Y = -ev.AccelerometerEvent.Y;
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
            dataStorage->Window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
            dataStorage->ViewController = [[HideStatusBarView alloc] init: this];
            dataStorage->Window.rootViewController = dataStorage->ViewController;
            [dataStorage->Window makeKeyAndVisible];
        }
    }

    void CIrrDeviceiOS::createViewAndDriver()
    {
        SIrrDeviceiOSDataStorage* dataStorage = static_cast<SIrrDeviceiOSDataStorage*>(DataStorage);

        switch (CreationParams.DriverType)
        {
            case video::EDT_OGLES2:
#ifdef _IRR_COMPILE_WITH_OGLES2_
                {
                    EAGLRenderingAPI OpenGLESVersion = kEAGLRenderingAPIOpenGLES2;
                    // For IOS we use 64bit only and all 64bit ios devices support GLES3 anyway
                    if (!CreationParams.ForceLegacyDevice)
                        OpenGLESVersion = kEAGLRenderingAPIOpenGLES3;
                    else
                        OpenGLESVersion = kEAGLRenderingAPIOpenGLES2;
                    dataStorage->m_eagl_context = [[EAGLContext alloc] initWithAPI:OpenGLESVersion];
                    [EAGLContext setCurrentContext:dataStorage->m_eagl_context];

                    CIrrViewiOS* view = [[CIrrViewiOS alloc] initWithFrame:[[UIScreen mainScreen] bounds]
                        forDevice:this forContext:dataStorage->m_eagl_context];
                    dataStorage->View = view;
                    m_native_scale = dataStorage->Window.screen.nativeScale;
                    view.contentScaleFactor = m_native_scale;
                    // This will initialize the default framebuffer, which bind its valus to GL_FRAMEBUFFER_BINDING
                    beginScene();
                    GLint default_fb = 0;
                    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_fb);

                    CreationParams.WindowSize =
                        {
                            (u32)view.drawableWidth,
                            (u32)view.drawableHeight
                        };
                    [dataStorage->Window addSubview:view];
                    VideoDriver = new video::COGLES2Driver(CreationParams, FileSystem, this, default_fb);

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

        dataStorage->ViewController.view = dataStorage->View;
    }
    void CIrrDeviceiOS::beginScene()
    {
        [static_cast<SIrrDeviceiOSDataStorage*>(DataStorage)->View bindDrawable];
    }
    void CIrrDeviceiOS::swapBuffers()
    {
        [static_cast<SIrrDeviceiOSDataStorage*>(DataStorage)->View display];
    }
    std::string CIrrDeviceiOS::getSystemLanguageCode()
    {
        NSString* language = [[NSLocale preferredLanguages] firstObject];
        return std::string([language UTF8String]);
    }
    void CIrrDeviceiOS::openURLiOS(const char* url)
    {
        UIApplication* application = [UIApplication sharedApplication];
        NSString* url_nsstring = [NSString stringWithCString:url encoding:NSUTF8StringEncoding];
        NSURL* nsurl_val = [NSURL URLWithString:url_nsstring];
        [application openURL:nsurl_val];
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
