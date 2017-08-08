// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_KEY_CODES_H_INCLUDED__
#define __IRR_KEY_CODES_H_INCLUDED__

namespace irr
{

	enum EKEY_CODE
	{
		IRR_KEY_UNKNOWN          = 0x0,
		IRR_KEY_LBUTTON          = 0x01,  // Left mouse button
		IRR_KEY_RBUTTON          = 0x02,  // Right mouse button
		IRR_KEY_CANCEL           = 0x03,  // Control-break processing
		IRR_KEY_MBUTTON          = 0x04,  // Middle mouse button (three-button mouse)
		IRR_KEY_XBUTTON1         = 0x05,  // Windows 2000/XP: X1 mouse button
		IRR_KEY_XBUTTON2         = 0x06,  // Windows 2000/XP: X2 mouse button
		IRR_KEY_BACK             = 0x08,  // BACKSPACE key
		IRR_KEY_TAB              = 0x09,  // TAB key
		IRR_KEY_CLEAR            = 0x0C,  // CLEAR key
		IRR_KEY_RETURN           = 0x0D,  // ENTER key
		IRR_KEY_SHIFT            = 0x10,  // SHIFT key
		IRR_KEY_CONTROL          = 0x11,  // CTRL key
		IRR_KEY_MENU             = 0x12,  // ALT key
		IRR_KEY_PAUSE            = 0x13,  // PAUSE key
		IRR_KEY_CAPITAL          = 0x14,  // CAPS LOCK key
		IRR_KEY_KANA             = 0x15,  // IME Kana mode
		IRR_KEY_HANGUEL          = 0x15,  // IME Hanguel mode (maintained for compatibility use KEY_HANGUL)
		IRR_KEY_HANGUL           = 0x15,  // IME Hangul mode
		IRR_KEY_JUNJA            = 0x17,  // IME Junja mode
		IRR_KEY_FINAL            = 0x18,  // IME final mode
		IRR_KEY_HANJA            = 0x19,  // IME Hanja mode
		IRR_KEY_KANJI            = 0x19,  // IME Kanji mode
		IRR_KEY_ESCAPE           = 0x1B,  // ESC key
		IRR_KEY_CONVERT          = 0x1C,  // IME convert
		IRR_KEY_NONCONVERT       = 0x1D,  // IME nonconvert
		IRR_KEY_ACCEPT           = 0x1E,  // IME accept
		IRR_KEY_MODECHANGE       = 0x1F,  // IME mode change request
		IRR_KEY_SPACE            = 0x20,  // SPACEBAR
		IRR_KEY_PRIOR            = 0x21,  // PAGE UP key
		IRR_KEY_NEXT             = 0x22,  // PAGE DOWN key
		IRR_KEY_END              = 0x23,  // END key
		IRR_KEY_HOME             = 0x24,  // HOME key
		IRR_KEY_LEFT             = 0x25,  // LEFT ARROW key
		IRR_KEY_UP               = 0x26,  // UP ARROW key
		IRR_KEY_RIGHT            = 0x27,  // RIGHT ARROW key
		IRR_KEY_DOWN             = 0x28,  // DOWN ARROW key
		IRR_KEY_SELECT           = 0x29,  // SELECT key
		IRR_KEY_PRINT            = 0x2A,  // PRINT key
		IRR_KEY_EXECUT           = 0x2B,  // EXECUTE key
		IRR_KEY_SNAPSHOT         = 0x2C,  // PRINT SCREEN key
		IRR_KEY_INSERT           = 0x2D,  // INS key
		IRR_KEY_DELETE           = 0x2E,  // DEL key
		IRR_KEY_HELP             = 0x2F,  // HELP key
		IRR_KEY_0                = 0x30,  // 0 key
		IRR_KEY_1                = 0x31,  // 1 key
		IRR_KEY_2                = 0x32,  // 2 key
		IRR_KEY_3                = 0x33,  // 3 key
		IRR_KEY_4                = 0x34,  // 4 key
		IRR_KEY_5                = 0x35,  // 5 key
		IRR_KEY_6                = 0x36,  // 6 key
		IRR_KEY_7                = 0x37,  // 7 key
		IRR_KEY_8                = 0x38,  // 8 key
		IRR_KEY_9                = 0x39,  // 9 key
		IRR_KEY_A                = 0x41,  // A key
		IRR_KEY_B                = 0x42,  // B key
		IRR_KEY_C                = 0x43,  // C key
		IRR_KEY_D                = 0x44,  // D key
		IRR_KEY_E                = 0x45,  // E key
		IRR_KEY_F                = 0x46,  // F key
		IRR_KEY_G                = 0x47,  // G key
		IRR_KEY_H                = 0x48,  // H key
		IRR_KEY_I                = 0x49,  // I key
		IRR_KEY_J                = 0x4A,  // J key
		IRR_KEY_K                = 0x4B,  // K key
		IRR_KEY_L                = 0x4C,  // L key
		IRR_KEY_M                = 0x4D,  // M key
		IRR_KEY_N                = 0x4E,  // N key
		IRR_KEY_O                = 0x4F,  // O key
		IRR_KEY_P                = 0x50,  // P key
		IRR_KEY_Q                = 0x51,  // Q key
		IRR_KEY_R                = 0x52,  // R key
		IRR_KEY_S                = 0x53,  // S key
		IRR_KEY_T                = 0x54,  // T key
		IRR_KEY_U                = 0x55,  // U key
		IRR_KEY_V                = 0x56,  // V key
		IRR_KEY_W                = 0x57,  // W key
		IRR_KEY_X                = 0x58,  // X key
		IRR_KEY_Y                = 0x59,  // Y key
		IRR_KEY_Z                = 0x5A,  // Z key
		IRR_KEY_LWIN             = 0x5B,  // Left Windows key (Microsoft® Natural® keyboard)
		IRR_KEY_RWIN             = 0x5C,  // Right Windows key (Natural keyboard)
		IRR_KEY_APPS             = 0x5D,  // Applications key (Natural keyboard)
		IRR_KEY_SLEEP            = 0x5F,  // Computer Sleep key
		IRR_KEY_NUMPAD0          = 0x60,  // Numeric keypad 0 key
		IRR_KEY_NUMPAD1          = 0x61,  // Numeric keypad 1 key
		IRR_KEY_NUMPAD2          = 0x62,  // Numeric keypad 2 key
		IRR_KEY_NUMPAD3          = 0x63,  // Numeric keypad 3 key
		IRR_KEY_NUMPAD4          = 0x64,  // Numeric keypad 4 key
		IRR_KEY_NUMPAD5          = 0x65,  // Numeric keypad 5 key
		IRR_KEY_NUMPAD6          = 0x66,  // Numeric keypad 6 key
		IRR_KEY_NUMPAD7          = 0x67,  // Numeric keypad 7 key
		IRR_KEY_NUMPAD8          = 0x68,  // Numeric keypad 8 key
		IRR_KEY_NUMPAD9          = 0x69,  // Numeric keypad 9 key
		IRR_KEY_MULTIPLY         = 0x6A,  // Multiply key
		IRR_KEY_ADD              = 0x6B,  // Add key
		IRR_KEY_SEPARATOR        = 0x6C,  // Separator key
		IRR_KEY_SUBTRACT         = 0x6D,  // Subtract key
		IRR_KEY_DECIMAL          = 0x6E,  // Decimal key
		IRR_KEY_DIVIDE           = 0x6F,  // Divide key
		IRR_KEY_F1               = 0x70,  // F1 key
		IRR_KEY_F2               = 0x71,  // F2 key
		IRR_KEY_F3               = 0x72,  // F3 key
		IRR_KEY_F4               = 0x73,  // F4 key
		IRR_KEY_F5               = 0x74,  // F5 key
		IRR_KEY_F6               = 0x75,  // F6 key
		IRR_KEY_F7               = 0x76,  // F7 key
		IRR_KEY_F8               = 0x77,  // F8 key
		IRR_KEY_F9               = 0x78,  // F9 key
		IRR_KEY_F10              = 0x79,  // F10 key
		IRR_KEY_F11              = 0x7A,  // F11 key
		IRR_KEY_F12              = 0x7B,  // F12 key
		IRR_KEY_F13              = 0x7C,  // F13 key
		IRR_KEY_F14              = 0x7D,  // F14 key
		IRR_KEY_F15              = 0x7E,  // F15 key
		IRR_KEY_F16              = 0x7F,  // F16 key
		IRR_KEY_F17              = 0x80,  // F17 key
		IRR_KEY_F18              = 0x81,  // F18 key
		IRR_KEY_F19              = 0x82,  // F19 key
		IRR_KEY_F20              = 0x83,  // F20 key
		IRR_KEY_F21              = 0x84,  // F21 key
		IRR_KEY_F22              = 0x85,  // F22 key
		IRR_KEY_F23              = 0x86,  // F23 key
		IRR_KEY_F24              = 0x87,  // F24 key
		IRR_KEY_NUMLOCK          = 0x90,  // NUM LOCK key
		IRR_KEY_SCROLL           = 0x91,  // SCROLL LOCK key
		IRR_KEY_LSHIFT           = 0xA0,  // Left SHIFT key
		IRR_KEY_RSHIFT           = 0xA1,  // Right SHIFT key
		IRR_KEY_LCONTROL         = 0xA2,  // Left CONTROL key
		IRR_KEY_RCONTROL         = 0xA3,  // Right CONTROL key
		IRR_KEY_LMENU            = 0xA4,  // Left MENU key
		IRR_KEY_RMENU            = 0xA5,  // Right MENU key
		IRR_KEY_BROWSER_BACK     = 0xA6,  // Browser Back key
		IRR_KEY_BROWSER_FORWARD  = 0xA7,  // Browser Forward key
		IRR_KEY_BROWSER_REFRESH  = 0xA8,  // Browser Refresh key
		IRR_KEY_BROWSER_STOP     = 0xA9,  // Browser Stop key
		IRR_KEY_BROWSER_SEARCH   = 0xAA,  // Browser Search key
		IRR_KEY_BROWSER_FAVORITES =0xAB,  // Browser Favorites key
		IRR_KEY_BROWSER_HOME     = 0xAC,  // Browser Start and Home key
		IRR_KEY_VOLUME_MUTE      = 0xAD,  // Volume Mute key
		IRR_KEY_VOLUME_DOWN      = 0xAE,  // Volume Down key
		IRR_KEY_VOLUME_UP        = 0xAF,  // Volume Up key
		IRR_KEY_MEDIA_NEXT_TRACK = 0xB0,  // Next Track key
		IRR_KEY_MEDIA_PREV_TRACK = 0xB1,  // Previous Track key
		IRR_KEY_MEDIA_STOP       = 0xB2,  // Stop Media key
		IRR_KEY_MEDIA_PLAY_PAUSE = 0xB3,  // Play/Pause Media key
		IRR_KEY_OEM_1            = 0xBA,  // for US    ";:"
		IRR_KEY_PLUS             = 0xBB,  // Plus Key   "+"
		IRR_KEY_COMMA            = 0xBC,  // Comma Key  ","
		IRR_KEY_MINUS            = 0xBD,  // Minus Key  "-"
		IRR_KEY_PERIOD           = 0xBE,  // Period Key "."
		IRR_KEY_OEM_2            = 0xBF,  // for US    "/?"
		IRR_KEY_OEM_3            = 0xC0,  // for US    "`~"
		IRR_KEY_OEM_4            = 0xDB,  // for US    "[{"
		IRR_KEY_OEM_5            = 0xDC,  // for US    "\|"
		IRR_KEY_OEM_6            = 0xDD,  // for US    "]}"
		IRR_KEY_OEM_7            = 0xDE,  // for US    "'""
		IRR_KEY_OEM_8            = 0xDF,  // None
		IRR_KEY_OEM_AX           = 0xE1,  // for Japan "AX"
		IRR_KEY_OEM_102          = 0xE2,  // "<>" or "\|"
		IRR_KEY_ATTN             = 0xF6,  // Attn key
		IRR_KEY_CRSEL            = 0xF7,  // CrSel key
		IRR_KEY_EXSEL            = 0xF8,  // ExSel key
		IRR_KEY_EREOF            = 0xF9,  // Erase EOF key
		IRR_KEY_PLAY             = 0xFA,  // Play key
		IRR_KEY_ZOOM             = 0xFB,  // Zoom key
		IRR_KEY_PA1              = 0xFD,  // PA1 key
		IRR_KEY_OEM_CLEAR        = 0xFE,   // Clear key

		IRR_KEY_CODES_COUNT      = 0xFF // this is not a key, but the amount of keycodes there are.
	};

} // end namespace irr

#endif

