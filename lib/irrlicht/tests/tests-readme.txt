
Welcome to the Irrlicht test suite.
===================================
This is composed of a series of tests which exercise basic Irrlicht
functionality.  These are not strictly unit tests, since there is no stub
framework that isolates each method under test.  They do however test small
units of functionality and should help to isolate problems and spot
regressions.

You are encouraged to run the tests whenever you make any significant code
change, and to add tests for any new or modified area of code.

The overall test application will return a count of the number of tests that
failed, i.e. 0 is success.  It will also log to tests/tests.log file, and on
success will update the tests/tests-last-passed-at.txt file (which gets
committed with code changes as a very basic verification that we're not
regressing).


Working directory
=================
Since the tests rely on the presence of /media and /empty/empty subdirectories,
the working directory must be the /tests directory, not /bin/$PLATFORM. This
means that you cannot run /bin/$PLATFORM/texts.exe from there.  You can however
cd to /tests and run ../bin/$PLATFORM/tests.exe


Adding a new test
=================
To add a new test, e.g. "myNewTest":

1) Create tests/myNewTest.cpp.  At a minimum, this must contain a function with
   the signature bool fnName(void), where fnName is the same as the filename
   (without the .cpp suffix).

   This function must return true if the tests passes, or false if it fails. In
   this example, the function should be:

   bool myNewTest(void)
   {
   ...
   }

2) Add myNewTest.cpp to the build targets in tests.cbp, tests_vc8.vcproj and
   tests_vc9.vcproj.  These are all text files that can be edited manually by
   hand; just copy, paste and modify an existing source file entry.

3) In tests/main.cpp, find the list of TEST() macro calls, and add a call to
   your new test, using the TEST macro, e.g.:

   TEST(myNewTest);

4) Run your test, and verify any images that it produces (see "Screenshots").

5) Remember to svn add tests/newNewTest.cpp and any new tests/media/ files.

Your test will be run independently in its own indepedent process. It is
responsible for creating any required resources or Irrlicht interfaces, and for
cleaning up after itself and restoring the working directory to /tests.  You do
not have to link against Irrlicht.lib; the whole application is already linked
to it.


Logging
=======
Please use logTestString() to log any interesting output or fails from your
test.  This is declared in tests/testUtils.h.  Its output goes to
tests/tests.log


Screenshots
===========
testUtils.h/.cpp provides a function to create a screenshot and compare it with
a reference image.  This is useful for validating new or changed functionality,
and for catching regressions.

Call the unambiguously named takeScreenshotAndCompareAgainstReference()
function to do this. It needs an IVideoDriver (which it will use to create the
first part of the filename) and a unique filename including an image format
suffix, e.g. "-myNewTest.png".  You should use .png as a suffix unless you have
a very specific need to use another format.  Please avoid using .jpg as image
compression can introduce artifacts and false fails.

Optionally, you can specify the amount of match that is required between the
produced screenshot and the reference image.  While the images should match
exactly, we have found that OpenGL implementations can vary significantly
across test machines, often around 99% match (of total colour values across all
pixels).  You may have to go as low as 98% for some images, but please try to
err on the side of strictness until we can determine that your test image needs
to be fuzzier on other peoples' machines.

If takeScreenshotAndCompareAgainstReference() can't find an existing reference
image, it will create one from the screenshot.  In effect, this means that you
have to run your test once (expecting it to fail) in order to produce the
initial reference images.  The new images are created in tests/results with
filename:

driverName-filename.suffix

e.g. OpenGL-myNewTest.png (note that the OpenGL driver elides the actual OpenGL
version number from the filename, as this tends to differ between machines and
installations).

You should check these new images carefully to ensure that they show exactly
what you expect.  Please do not just assume that they do, as validating bad
behaviour is worse than not validating it at all!

If the images do appear exactly as you expect, move them to the tests/media
directory, and re-run the tests.  They should now pass.  Remember to svn add
any new media files!


What to do when the tests fail
==============================
DON'T PANIC!

This is a Good Thing.  Failing tests challenge our assumptions and help us to
make Irrlicht more robust.

First, check your working directory.  The tests need to be run from the tests/
directory, not a /bin subdirectory.  You can do this using the working
directory in your debugger, or on the command line by running the tests
executable (wherever it is build) from the tests/ directory.

If you need to debug a test, first move it temporarily to the start of the list
of TEST() macros. This is because each test runs in its own process, so only
the first test will have the debugger attached.

If the fail is due to a bitmap difference, carefully compare the bitmaps, and
the amount of failure. The OpenGL device does tend to produce differences.  You
should not just automatically make a test fuzzier, but if you can rule out any
real issue in the code, it can be valid to accept OpenGL image matches as low
as 98%.  Other devices should not require this amount of fuzziness!

If you can't figure out the reason for the failure (or better yet, if you can,
and think the tests and/or Irrlicht need updated), then please do raise the
issue in the bug forum:

http://irrlicht.sourceforge.net/phpBB2/viewforum.php?f=7

We do want to hear about fails, and will thank you for finding them.

Running specific tests
======================
The app takes two parameters. First is the test to start with (starting at 0
anddefaulting to 0), the second is the number of tests to run (beginning with
the one given as first parameter). If the second parameter is not given, all
existing tests are run (again starting with the first parameter). So, starting
the test suite without a parameter will really run all tests.  Another special
parameter is '--list', which outputs a list of all existing tests and their
respective number.

For debugging purposes it can make sense to run a test without spawning a
separate process for each test case. This can be switched off by a boolean flag
in main.cpp ('spawn=false').

Currently implemented tests
===========================
000. disambiguateTextures
001. testIrrArray
002. testIrrMap
003. testIrrList
004. exports
005. irrCoreEquals
006. testIrrString
007. testLine2d
008. matrixOps
009. testDimension2d
010. testVector2d
011. testVector3d
012. testQuaternion
013. testS3DVertex
014. testaabbox3d
015. color
016. testTriangle3d
017. vectorPositionDimension2d
018. filesystem
019. archiveReader
020. testXML
021. serializeAttributes
022. fast_atof
023. loadTextures
024. collisionResponseAnimator
025. enumerateImageManipulators
026. removeCustomAnimator
027. sceneCollisionManager
028. sceneNodeAnimator
029. meshLoaders
030. testTimer
031. softwareDevice
032. b3dAnimation
033. burningsVideo
034. billboards
035. createImage
036. cursorSetVisible
037. flyCircleAnimator
038. guiDisabledMenu
039. makeColorKeyTexture
040. md2Animation
041. meshTransform
042. skinnedMesh
043. testGeometryCreator
044. writeImageToFile
045. ioScene
046. videoDriver
047. screenshot
048. drawPixel
049. drawRectOutline
050. drawVertexPrimitive
051. material
052. renderTargetTexture
053. textureFeatures
054. textureRenderStates
055. transparentMaterials
056. userclipplane
057. antiAliasing
058. draw2DImage
059. lights
060. twodmaterial
061. viewPort
062. mrt
063. projectionMatrix
064. planeMatrix
065. terrainSceneNode
066. lightMaps
067. triangleSelector

