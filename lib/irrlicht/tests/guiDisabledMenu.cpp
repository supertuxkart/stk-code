// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace gui;

// Tests that disabled GUI menu items don't cause their submenu to appear when hovered over.
/**
	http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?p=178436#178436
 */

bool guiDisabledMenu(void)
{
	IrrlichtDevice *device = createDevice( video::EDT_BURNINGSVIDEO,
											dimension2d<u32>(160, 40), 32);
	assert_log(device);
	if (!device)
		return false;

	video::IVideoDriver* driver = device->getVideoDriver();
	gui::IGUIEnvironment* env = device->getGUIEnvironment();

	gui::IGUIContextMenu* menu = env->addMenu();
	menu->addItem(L"Menu", -1, true, true);
	gui::IGUIContextMenu* subMenu = menu->getSubMenu(0);
	subMenu->addItem(L"Submenu 1", -1, false, true);
	gui::IGUIContextMenu* subSubMenu = subMenu->getSubMenu(0);
	subSubMenu->addItem(L"Final item");

	SEvent event;
	event.EventType = EET_MOUSE_INPUT_EVENT;
	event.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
	event.MouseInput.X = menu->getAbsolutePosition().UpperLeftCorner.X + 1;
	event.MouseInput.Y = menu->getAbsolutePosition().UpperLeftCorner.Y + 1;
	(void)menu->OnEvent(event);

	// Hovering over the disabled submenu shouldn't cause the "Final item" to appear.
	event.MouseInput.Event = EMIE_MOUSE_MOVED;
	event.MouseInput.X = subMenu->getAbsolutePosition().UpperLeftCorner.X + 40;
	event.MouseInput.Y = subMenu->getAbsolutePosition().UpperLeftCorner.Y + 10;
	(void)menu->OnEvent(event);

	device->run();
	driver->beginScene(true, true, video::SColor(150,50,50,50));
	env->drawAll();
	driver->endScene();

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-guiDisabledMenu.png", 98.77f);
	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

