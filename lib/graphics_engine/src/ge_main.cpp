#include "ge_main.hpp"

namespace GE
{
irr::video::IVideoDriver* g_driver = NULL;
GEConfig g_config = {};

void init(irr::video::IVideoDriver* driver)
{
    g_driver = driver;
}

irr::video::IVideoDriver* getDriver()
{
    return g_driver;
}

GEConfig* getGEConfig()
{
    return &g_config;
}

void deinit()
{
}

}
