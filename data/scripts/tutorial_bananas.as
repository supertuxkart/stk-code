void onTrigger()
{
    displayMessage("Bananas! Bananas! Everywhere!");
    squashKart(0,35.0); //id of kart,time to squash
    //teleportKart(0, 0, 0 ,0); //id of kart, x,y,z
    //setVelocity(0, 5, 50 , 8); //id of kart, velocity 								//components x,y,z
    jumpKartTo(0, 0, 0); //id of kart, target x,y
    createTrigger("added_script",0,0,0,30.0);                    	//name,x,y,z,trigger distance

}
