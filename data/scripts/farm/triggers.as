void haybail()
{
/*to activate this add the following line to stk-assets/farm/scene.xml

<object type="action-trigger" action="haybail" distance="30.0" xyz="100.72 10.20 -26.22" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>

*/
    displayMessage("Haybail deactivated");
    //disableAnimation("hayBail.b3d");
	TrackObject @t_obj = getTrackObject("hayBail.b3d");
     //t_obj.setEnable(false);
     PhysicalObject @haybail = t_obj.getPhysicalObject();
     haybail.disable();
     Mesh @haybailMesh = t_obj.getMesh();
     Animator @haybailAnimator = t_obj.getAnimator();
     haybailAnimator.setPaused(true);
     //if (haybail.isFlattener())squashKart(0,35.0);
}

void haybail_deactivate()
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


