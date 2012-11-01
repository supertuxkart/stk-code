// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;

class CustomAnimator : public ISceneNodeAnimator
{
	void animateNode(ISceneNode* node, u32 timeMs)
	{
		// Check that I can remove myself from my node durings its animateNode() loop.
		node->removeAnimator(this);
	}

	ISceneNodeAnimator* createClone(ISceneNode* node, ISceneManager* newManager=0) { return 0 ; }
};


/** Test that a custom animator can remove itself cleanly from an ISceneNode during its
 *  own animateNode() loop.
 * http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=32271 */
bool removeCustomAnimator(void)
{
	IrrlichtDevice * device = irr::createDevice(video::EDT_NULL, dimension2du(160, 120));
	assert_log(device);
	if(!device)
		return false;

	ISceneManager * smgr = device->getSceneManager();

	ISceneNode * node = smgr->addEmptySceneNode();
	CustomAnimator * instantlyElapsing1 = new CustomAnimator();
	CustomAnimator * instantlyElapsing2 = new CustomAnimator();
	node->addAnimator(instantlyElapsing1);
	node->addAnimator(instantlyElapsing2);

	// This should result in both custom animators being removed and
	// deleted cleanly, without a crash.
	node->OnAnimate(0);

	device->closeDevice();
	device->run();
	device->drop();

	// If we didn't crash, then the test passed.
	return true;
}


