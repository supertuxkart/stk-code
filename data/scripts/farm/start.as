void onStart()
{
    displayMessage("Track Loaded");
    
    Vec3 creationloc = Vec3(69.97 ,8.08 ,-107.84);
    Vec3 creationloc2 = Vec3(100.72, 10.20,-26.22);
    createTrigger("haybail_deactivate", creationloc, 20.00); 
    createTrigger("haybail",creationloc2 , 30.00);  

     TrackObject @t_obj = getTrackObject("cow");
     SoundEmitter @cowmoo = t_obj.getSoundEmitter();
     Vec3 newlocation = Vec3(0,2,5);
     cowmoo.move(newlocation);
     teleportKart(0,newlocation);

}

