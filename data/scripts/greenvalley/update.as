void onUpdate()
{

     TrackObject @t_obj = getTrackObject("anim_sheep2.b3d");
     Mesh @sheepMesh = t_obj.getMesh();
     Vec3 newloc = Vec3(2,-2,-2);
     sheepMesh.move(newloc);
     if (sheepMesh.getCurrentFrame()==2){
     sheepMesh.setLoop(8,48);
     sheepMesh.setCurrentFrame(9);
     }
     Vec3 speed = Vec3(10,0,0);
     setVelocity(0,speed);
}
