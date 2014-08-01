void onStart()
{

    displayMessage("Track Loaded");
    //For green valley sheep test. See sheep_approach.as
    Vec3 newloc = Vec3(-5.48,7.21,0.49);
    createTrigger("sheep_approach",newloc,20.0);

}
