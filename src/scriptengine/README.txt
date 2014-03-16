This is currently only a tiny prototype to get an idea of what the scripting engine can do/ check it's performance.
TODO:
set up a build target for angelscript
get file manager support for loading scripts
move scripts to stk-data, etc.


Steps to get this working.
1)get AngelScript SDK 2.28.1

http://www.angelcode.com/angelscript/downloads.html

2)Build the AngelScript library
IF LINUX{

Build AngelScript by going to 

cd ANGELSCRIPT_DIR\sdk\angelscript\projects\gnuc
make

==> angelscript.a
==>change stk-code/cmakefilelists.txt (ctrl + f "angelscript)
==>change stk-code/src/scriptengine/scriptengine.cpp line 254 as appropriate
}

IF WINDOWS{

Load the VS Project, Build the .lib

==>angelscriptd.lib
==>change stk/cmakefilelists.txt (ctrl + f "angelscript)
==>change stk-code/src/scriptengine/scriptengine.cpp line 254 as appropriate
}

Copy the generated library file (.a or .lib) to stk-code/dependencies/lib
Comment/Uncomment the lines in CMakeLists in stk-code as necessary (for Linux/Windows)

Build the game.
Test by changing the strings in stk-code/src/tracks/



Steps for creating new scripts/ actions
Add an action trigger to the relevant track (Either add it on blender and export it / set it up in scene.xml)
Create a new script with the defined action scene.xml for the action trigger created. (For example, for tutorial_bananas =>

  <object type="action-trigger" action="tutorial_bananas" distance="7.0" xyz="67.90 0.86 99.49" hpr="0.0 -0.0 0.0" scale="7.00 7.00 7.00"/>

action is "tutorial_bananas"
so the script it calls is, stk/stk-code/scriptengine/tutorial_bananas.as

Make sure the method onTrigger() is defined. Check tutorial_bananas.as for more details

