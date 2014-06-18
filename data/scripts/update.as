void onUpdate()
{

     TrackObject @t_obj = getTrackObject("anim_sheep2.b3d");
     Mesh @sheepMesh = t_obj.getMesh();
     if (sheepMesh.getCurrentFrame()==2){
     sheepMesh.setLoop(8,48);
     sheepMesh.setCurrentFrame(9);
     }
}
