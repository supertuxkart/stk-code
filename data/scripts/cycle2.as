void onTrigger()
{
/*to activate these 3 scripts add the following lines to stk-assets/20_harvest/scene.xml

  <object type="action-trigger" action="cycle1" distance="30.0" xyz="139.86 10.21 80.94" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>
  <object type="action-trigger" action="cycle2" distance="30.0" xyz="110.86 10.21 50.94" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>
  <object type="action-trigger" action="cycle3" distance="30.0" xyz="-1.69 10.60 -42.19" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>
 
*/
    displayMessage("2 triggered, 3 activated, 1 deactivated");
    enableTrigger("cycle3");
    disableTrigger("cycle1");

}
