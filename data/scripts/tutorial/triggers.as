void added_script()
{
    /*displayMessage(getKeyBinding(PA::FIRE) + getKeyBinding(PA::ACCEL) + "This trigger was added by another script");
    jumpKartTo( 0, 67.90, 99.49 );
    Vec3 a;
    Vec3 b;
    b=a;
    Vec3 c = Vec3();
    Vec3 d = Vec3(2,3,4);
    printVec3(d); */
}


void tutorial_bananas()
{
    displayMessage("Bananas! Bananas! Everywhere!");
    squashKart(0,35.0); //id of kart,time to squash
    //teleportKart(0, 0, 0 ,0); //id of kart, x,y,z
    //setVelocity(0, 5, 50 , 8); //id of kart, velocity 								//components x,y,z
    jumpKartTo(0, 0, 0); //id of kart, target x,y
    createTrigger("added_script",0,0,0,30.0);                    	//name,x,y,z,trigger distance

}

void tutorial_drive(){

	displayMessage("Accelerate with <" +
				getKeyBinding(PA::ACCEL) +
				"> and steer with <" +
				getKeyBinding(PA::STEER_LEFT) +
				"> and <" +
				getKeyBinding(PA::STEER_RIGHT) + ">");
}

void tutorial_giftboxes(){

	displayMessage("Collect gift boxes, and fire the weapon with <" + getKeyBinding(PA::FIRE) + "> to blow away these boxes");
}

void tutorial_backgiftboxes()
{
    displayMessage("Press <" + getKeyBinding(PA::LOOK_BACK) + "> to look behind. Fire the weapon with <" + getKeyBinding(PA::FIRE) + "> while pressing <" + getKeyBinding(PA::LOOK_BACK) + "> to fire behind!");
}

void tutorial_nitro_use()
{
    displayMessage("Use the nitro you collected by pressing <" + getKeyBinding(PA::NITRO) + ">!");
}

void tutorial_nitro_collect()
{
    displayMessage("Collect nitro bottles (we will use them after the curve)");
}

void tutorial_rescue()
{
    displayMessage("Oops! When you're in trouble, press <" + getKeyBinding(PA::RESCUE) + "> to be rescued");

}

void tutorial_skidding()
{
    displayMessage("Accelerate and press the <" + getKeyBinding(PA::DRIFT) + "> key while turning to skid. Skidding for a short while can help you turn faster to take sharp turns");

}

void tutorial_skidding2()
{
    displayMessage("Note that if you manage to skid for several seconds, you will receive a bonus speedup as a reward!");
}

void tutorial_endmessage()
{
    displayMessage("You are now ready to race. Good luck!");
}

