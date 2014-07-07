void onStart()
{
    displayMessage("Track Loaded");
    //For green valley sheep test. See sheep_approach.as
    createTrigger("haybail_deactivate", 69.97 ,8.08 ,-107.84, 20.00); //follows xzy for now
    createTrigger("haybail",100.72, 10.20,-26.22 , 30.00);  

     TrackObject @t_obj = getTrackObject("cow");
     SoundEmitter @cowmoo = t_obj.getSoundEmitter();
     Vec3 newlocation = Vec3(0,2,5);
     cowmoo.move(newlocation);

}

