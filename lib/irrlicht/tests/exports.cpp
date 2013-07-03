// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

//! Tests that symbols exported from Irrlicht can be used by the user app.
bool exports(void)
{
	logTestString("Checking whether IdentityMatrix is exported.\n");
	irr::core::matrix4 identity = irr::core::IdentityMatrix;
	(void)identity; // Satisfy the compiler that it's used.

	irr::video::SMaterial id = irr::video::IdentityMaterial;
	(void)id; // Satisfy the compiler that it's used.
	// If it built, we're done.
	return true;
}
