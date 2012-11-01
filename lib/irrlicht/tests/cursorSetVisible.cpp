// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;

struct TrapMouseMoves : IEventReceiver
{
	int MouseMovesReceived;

	TrapMouseMoves() : MouseMovesReceived(0) { }

	virtual bool OnEvent(const SEvent & event)
	{
		if(event.EventType == EET_MOUSE_INPUT_EVENT
			&& event.MouseInput.Event == EMIE_MOUSE_MOVED)
			MouseMovesReceived++;

		return false;
	}
};

/** This test verifies that setting the cursor visibility
	only generates a mouse message when it actually changes */
bool cursorSetVisible(void)
{
	IrrlichtDevice * device = createDevice(video::EDT_SOFTWARE, dimension2d<u32>(1, 1));
	TrapMouseMoves moveTrapper;
	device->setEventReceiver(&moveTrapper);

	device->run();

	gui::ICursorControl * cursor = device->getCursorControl();

	// Move the cursor inside the Irrlicht window so that we get messages for it.
	cursor->setPosition(0, 0);
	device->run(); // Receive any messages

	cursor->setVisible(false); // Should generate a mouse move
	device->run(); // Receive any messages

	cursor->setVisible(false); // Should not generate a mouse move
	device->run(); // Receive any messages

	cursor->setVisible(true); // Should generate a mouse move
	device->run(); // Receive any messages

	cursor->setVisible(true); // Should not generate a mouse move
	device->run(); // Receive any messages


	// We should get at most 3 messages: one for the setPosition(), and one for
	// each actual change of visibility.
	bool result = (moveTrapper.MouseMovesReceived <= 3);

	device->closeDevice();
	device->run();
	device->drop();

	if(!result)
	{
		logTestString("ERROR: cursorSetVisible received %d events.\n", moveTrapper.MouseMovesReceived);
		assert_log(false);
	}

	return result;
}

