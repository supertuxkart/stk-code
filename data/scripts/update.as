void onUpdate()
{

     TrackObject @t_obj = getTrackObject("anim_sheep2.b3d");
     //t_obj.setEnable(false);
     Mesh @sheepMesh = t_obj.getMesh();
     //displayMessage("moo");
     //sheepMesh.setLoop(1,1); //rapid-nod sheep
     if (sheepMesh.getCurrentFrame()>=0)sheepMesh.setLoop(1,15);

}
