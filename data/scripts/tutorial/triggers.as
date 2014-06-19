void added_script()
{
    displayMessage("This trigger was added by another script");
    jumpKartTo( 0, 67.90, 99.49 );
    Vec3 a;
    Vec3 b;
    b=a;
    Vec3 c = Vec3();
    Vec3 d = Vec3(2,3,4);
    printVec3(d);
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

