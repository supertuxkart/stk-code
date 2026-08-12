#ifndef HEADER_LINUX_TOUCH_DETECT_HPP
#define HEADER_LINUX_TOUCH_DETECT_HPP

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#endif

/** Linux /proc and DMI helpers for touchscreen vs keyboard.
 *  Used by Irrlicht SDL (supportsTouchDevice) and STK (touch-only policy). */
namespace LinuxTouchDetect
{
    const int KEY_A = 30;
    const int ABS_MT_POSITION_X = 53;
    const int INPUT_PROP_DIRECT = 1;

    inline bool containsI(const char* hay, const char* needle)
    {
        if (!hay || !needle || !needle[0])
            return false;
        const size_t nlen = std::strlen(needle);
        const size_t hlen = std::strlen(hay);
        if (nlen > hlen)
            return false;
        for (size_t i = 0; i + nlen <= hlen; i++)
        {
            size_t j = 0;
            for (; j < nlen; j++)
            {
                if (std::tolower((unsigned char)hay[i + j]) !=
                    std::tolower((unsigned char)needle[j]))
                    break;
            }
            if (j == nlen)
                return true;
        }
        return false;
    }

    inline bool ignoredKeyboardName(const char* name)
    {
        if (!name || !name[0])
            return true;
        return containsI(name, "power button") ||
               containsI(name, "sleep button") ||
               containsI(name, "lid switch") ||
               containsI(name, "video bus") ||
               containsI(name, "gpio-keys") ||
               containsI(name, "headset") ||
               containsI(name, "hdmi") ||
               containsI(name, "sof-hda") ||
               containsI(name, "consumer control");
    }

    inline bool bitmapHasBit(const char* hex, unsigned bit)
    {
        unsigned long words[32];
        int n = 0;
        int word_bits = 32;
        const char* p = hex;
        std::memset(words, 0, sizeof(words));
        while (p && *p && n < 32)
        {
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
                p++;
            if (!*p)
                break;
            const char* start = p;
            char* end = NULL;
            words[n] = std::strtoul(p, &end, 16);
            if (end == p)
                break;
            if ((int)(end - start) > 8)
                word_bits = 64;
            n++;
            p = end;
        }
        if (n == 0)
            return false;
        const unsigned word_from_low = bit / (unsigned)word_bits;
        const unsigned bit_in_word = bit % (unsigned)word_bits;
        const int idx = n - 1 - (int)word_from_low;
        if (idx < 0 || idx >= n)
            return false;
        return (words[idx] & (1UL << bit_in_word)) != 0;
    }

    inline void scanProcBusInput(bool* has_touch, bool* has_keyboard)
    {
        FILE* f = std::fopen("/proc/bus/input/devices", "r");
        if (!f)
            return;
        char line[512];
        char name[256];
        unsigned prop = 0;
        bool key_a = false;
        bool abs_mt = false;
        name[0] = 0;
        while (std::fgets(line, sizeof(line), f))
        {
            if (std::strncmp(line, "N: Name=\"", 9) == 0)
            {
                name[0] = 0;
                std::sscanf(line, "N: Name=\"%255[^\"]\"", name);
            }
            else if (std::strncmp(line, "B: PROP=", 8) == 0)
                prop = (unsigned)std::strtoul(line + 8, NULL, 16);
            else if (std::strncmp(line, "B: KEY=", 7) == 0)
                key_a = bitmapHasBit(line + 7, KEY_A);
            else if (std::strncmp(line, "B: ABS=", 7) == 0)
                abs_mt = bitmapHasBit(line + 7, ABS_MT_POSITION_X);
            else if (line[0] == '\n' || line[0] == '\r' || line[0] == 0)
            {
                if ((prop & (1u << INPUT_PROP_DIRECT)) ||
                    containsI(name, "touchscreen"))
                    *has_touch = true;
                else if (abs_mt && !(prop & 1u) && containsI(name, "touch"))
                    *has_touch = true;
                if (key_a && !ignoredKeyboardName(name))
                    *has_keyboard = true;
                name[0] = 0;
                prop = 0;
                key_a = false;
                abs_mt = false;
            }
        }
        std::fclose(f);
    }

    inline int chassisType()
    {
        FILE* f = std::fopen("/sys/class/dmi/id/chassis_type", "r");
        if (!f)
            return -1;
        int type = -1;
        if (std::fscanf(f, "%d", &type) != 1)
            type = -1;
        std::fclose(f);
        return type;
    }

    inline bool osReleaseHas(const char* needle)
    {
        FILE* f = std::fopen("/etc/os-release", "r");
        if (!f)
            return false;
        char line[512];
        while (std::fgets(line, sizeof(line), f))
        {
            if (containsI(line, needle))
            {
                std::fclose(f);
                return true;
            }
        }
        std::fclose(f);
        return false;
    }

    inline bool isUbuntuTouch()
    {
        if (osReleaseHas("Ubuntu Touch") || osReleaseHas("UBUNTU_TOUCH") ||
            osReleaseHas("VARIANT_ID=touch") || osReleaseHas("lomiri"))
            return true;
#ifndef _WIN32
        if (access("/usr/share/ubports", F_OK) == 0)
            return true;
#endif
        const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
        if (desktop && (containsI(desktop, "Lomiri") || containsI(desktop, "Unity8")))
            return true;
        const char* click = std::getenv("CLICK_FRAMEWORK");
        return click && click[0];
    }

    inline bool hasTouchscreen()
    {
        bool touch = false;
        bool keyboard = false;
        scanProcBusInput(&touch, &keyboard);
        if (isUbuntuTouch())
            touch = true;
        return touch;
    }

    inline bool hasHardwareKeyboard()
    {
        bool touch = false;
        bool keyboard = false;
        scanProcBusInput(&touch, &keyboard);
        if (isUbuntuTouch())
            return false;
        const int chassis = chassisType();
        if (chassis == 11 || chassis == 30)
            return false;
        return keyboard;
    }

    inline bool isTouchOnly()
    {
        if (isUbuntuTouch())
            return true;
        const int chassis = chassisType();
        bool touch = hasTouchscreen();
        bool keyboard = hasHardwareKeyboard();
        if ((chassis == 11 || chassis == 30) && touch)
            return true;
        return touch && !keyboard;
    }
}

#endif
