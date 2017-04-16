## Building SuperTuxKart on Windows

1. Install Visual Studio 2013 (or later). The free express versions work fine.

2. Download and install a source package - either a released package or from our [git/svn repositories](https://supertuxkart.net/Source_control).
3. Download the latest dependency package from [here](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/Windows/). Unzip it in the root directory, so that the dependencies directory is next to the src and data directories (if you are updating from a previous dependency package, you can delete the .dll files in the root directory, they are not needed anymore).
4. Download cmake and install it. Then start cmake-gui and select the STK root directory as 'Where is the source code', and a new directory in the root directory (next to src, data etc) as the build directory (for now I assume that this directory is called bld).
5. Click on configure. You will be asked to create the directory (yes), then for your VS version. Make sure you select the right version (be aware of the easy to confuse version numbers: VS 2013 = version 12). Click on configure, then generate. This will create the directory 'bld', and a VS solution in that directory.
6. In Visual Studio open the project file generated in the 'bld' folder.
7. Right click on the supertuxkart project in the solution explorer, and select "Set as StartUp Project".
8. Select Build->Build Solution (or press F7) to compile.

## Building SuperTuxKart on Windows, with Visual Studio 2017 - method number 2
To Build SuperTuxKart on Windows, follow these instructions for the 2nd method:

1. Download and install Visual Studio from here: [Visual Studio - Download](https://www.visualstudio.com/downloads/), or from the Microsoft Download Center. The free Visual Studio Community edition works fine.

2. Download the SuperTuxKart source package from either [SuperTuxKart download area - SourceForge.net](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart/0.9.2) or [SuperTuxKart.net - Source Control](https://supertuxkart.net/Source_control), and unpack it.
3. Download the Windows dependencies package from either [SuperTuxKart download area: Dependecies - SourceForge.net](https://sourceforge.net/projects/supertuxkart/files/SuperTuxKart%20Dependencies/Windows/)
or [SuperTuxKart on GitHub - Dependencies](https://github.com/supertuxkart/dependencies), and unpack it; then copy either the `windows` or `windows_64bit` to the `stk-code` directory
4. Download CMake from here: [CMake - download page](https://cmake.org/download/), install it; once CMake is installed, double click on the CMake icon on your desktop, and point it towards your `stk-code` directory in the 'Where is the source code' field, and point it to a directory called `build` or `bld`
5. Press 'Configure'; CMake will ask you if it is OK to create the aformentioned directory, press `Yes`. CMake will then ask you about your version of Visual Studio. 
Confirm your selection; CMake will begin creating the required files for the build in the directory.
*If CMake fails to find the dependencies during configuration; you'll need to manually add them in the missing fields*
6. Navigate to your build directory and open the `SuperTuxKart.sln` file; Visual Studio will now load the solution. 
7. In the 'Solution Explorer', right click on the `supertuxkart` project and select "Set as StartUp project"
8. Open the 'Build' menu and select 'Build Solution'; or, press `CTRL + SHIFT + B` - your solution's build process has begun.

*Note: To avoid confusion between releases and versions, refer to this table:*

Visual Studio Release | Version
----------------------|------------
Visual Studio 2017| 15
Visual Studio 2015| 14
Visual Studio 2013| 13