#import <AppKit/AppKit.h>

extern "C" void enable_momentum_scroll()
{
    [[NSUserDefaults standardUserDefaults] setBool: YES forKey: @"AppleMomentumScrollSupported"];
}
