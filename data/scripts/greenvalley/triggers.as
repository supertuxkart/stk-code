void sheep_dance()
{

	TrackObject @t_obj = getTrackObject("anim_sheep2.b3d");
     //t_obj.setEnable(false);
     Mesh @sheepMesh = t_obj.getMesh();
     //displayMessage("moo");
     sheepMesh.setLoop(1,3); //rapid-nod sheep
     //runScript("sheep_approach");
}


void sheep_approach()
{

	//TrackObject @t_obj = getTrackObject("anim_sheep2.b3d");
     //t_obj.setEnable(false);
     //Mesh @sheepMesh = t_obj.getMesh();
     //displayMessage("moo");
     //sheepMesh.setLoop(6,9); //rapid-nod sheep
     runScript("sheep_dance");
     
}

