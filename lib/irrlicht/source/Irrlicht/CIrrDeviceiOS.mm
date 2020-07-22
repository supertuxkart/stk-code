// Copyright (C) 2002-2008 Nikolaus Gebhardt
// Copyright (C) 2008 Redshift Software, Inc.
// Copyright (C) 2012 Patryk Nadrowski
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "SDL.h"
#include "SDL_syswm.h"

#import "CIrrDeviceiOS.h"
#import <sys/utsname.h>
#import <Foundation/NSString.h>
#import <Foundation/NSURL.h>
#import <UIKit/UIKit.h>

extern void getConfigForDevice(const char* dev);
extern void override_default_params_for_mobile();
extern int ios_main(int argc, char *argv[]);

std::string irr::CIrrDeviceiOS::getSystemLanguageCode()
{
    NSString* language = [[NSLocale preferredLanguages] firstObject];
    return std::string([language UTF8String]);
}
void irr::CIrrDeviceiOS::openURLiOS(const char* url)
{
    UIApplication* application = [UIApplication sharedApplication];
    NSString* url_nsstring = [NSString stringWithCString:url encoding:NSUTF8StringEncoding];
    NSURL* nsurl_val = [NSURL URLWithString:url_nsstring];
    [application openURL:nsurl_val];
}
void irr::CIrrDeviceiOS::debugPrint(const char* line)
{
    NSString* ns = [NSString stringWithUTF8String:line];
    NSLog(@"%@", ns);
}

extern "C" void init_objc(SDL_SysWMinfo* info, float* top, float* bottom, float* left, float* right)
{
    if (@available(iOS 11.0, *))
    {
        *top = info->info.uikit.window.safeAreaInsets.top,
        *bottom = info->info.uikit.window.safeAreaInsets.bottom,
        *left = info->info.uikit.window.safeAreaInsets.left,
        *right = info->info.uikit.window.safeAreaInsets.right;
    }
}

#ifdef main
#undef main
#endif
int main(int argc, char *argv[])
{
    override_default_params_for_mobile();
    struct utsname system_info;
    uname(&system_info);
    NSString* model = [NSString stringWithCString:system_info.machine
        encoding:NSUTF8StringEncoding];
    getConfigForDevice([model UTF8String]);
    return SDL_UIKitRunApp(argc, argv, ios_main);
}
