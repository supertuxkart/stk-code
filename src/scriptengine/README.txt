This is currently only a tiny prototype to get an idea of what the scripting engine can do/ check it's performance.
TODO:
get file manager support for loading scripts
move scripts to stk-data, etc.


Steps to get this working.

==>change stk-code/src/scriptengine/scriptengine.cpp line 254 as appropriate
(add the required directory)
Build the game.
Play the tutorial
Test by changing the strings in stk-code/src/tracks/tutorial_bananas.as




Steps for creating new scripts/ actions
Add an action trigger to the relevant track (Either add it on blender and export it / set it up in scene.xml)
Create a new script with the defined action scene.xml for the action trigger created. (For example, for tutorial_bananas =>

  <object type="action-trigger" action="tutorial_bananas" distance="7.0" xyz="67.90 0.86 99.49" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>

action is "tutorial_bananas"
so the script it calls is, stk/stk-code/scriptengine/tutorial_bananas.as

Make sure the method onTrigger() is defined. Check tutorial_bananas.as for more details

