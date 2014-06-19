void onStart()
{

    displayMessage("Track Loaded");
    //For green valley sheep test. See sheep_approach.as
    createTrigger("haybail_deactivate", 69.97 ,8.08 ,-107.84, 20.00); //follows xzy for now
    createTrigger("haybail",100.72, 10.20,-26.22 , 30.00);  

}

