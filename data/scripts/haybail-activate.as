void onTrigger()
{
/*to activate this add the following line to stk-assets/farm/scene.xml

<object type="action-trigger" action="haybail-activate" distance="10.0" xyz="69.97 8.08 -107.84" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>

*/
    displayMessage("Haybail reactivated");
    //enableAnimation("hayBail.b3d");
    squashKart(0,35.0); //id of kart,time to squash
    TrackObject @t_obj = getTrackObject("hayBail.b3d");
    Animator @haybailAnimator = t_obj.getAnimator();
    haybailAnimator.setPaused(false);
}
