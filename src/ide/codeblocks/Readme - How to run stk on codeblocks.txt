====================================================================================

Steps:
* Go to src->ide->codeblocks and open "supertuxkart workspace" file.
* Codeblocks should open automatically with 4 projects under the workspace.
* Build all the 4 projects in the following order:
	1. enet
	2. bullet_lib
	3. Irricht
	4. Supertuxkart
( You have to right click on each of the project in the project workspace 
and select "build")
* Select target as "debug" for debug version and "release" for release version
* To do this:
	1. Right-click and activate the project
	2. Go to "Build -> Select target" in the main menu and choose debug or release

* Make sure you have all the dependancy files extracted in root folder of stk
( where you will find folders named src and data)
* Your "dependancies" folder and the .dll files should be here

* Right-click and activate project "supertuxkart" and select 
	a. Build and Run for release version
	b. Start Debugging for debug version
	

[ If you get error "Irricht.dll" not found or problem in dynamic linking
	1. Go to "..\..\lib\Win32-gcc" and copy the "Irricht.dll" file to root folder
	* You will find this file after you have built you "Irricht" project ]
====================================================================================

Dependancies:
* Download "dependencies_for_0.8" and "dependencies_for_0.8_mingw"
* Extract both in the root stk folder
* In dependancies folder, cut everything from inside the include and lib folder
and paste it inside the dependancies folder itself

* Also copy libwin2_32.a and libwinmm.a to dependancies folder

====================================================================================
Tested on codeblocks 10.05 with mingw 4.4.1 compiler
====================================================================================
====================================================================================