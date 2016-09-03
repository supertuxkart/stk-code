# Optimizing SuperTuxKart shaders
A shader optmizer takes the un-optimized shaders code and does optimization like function inlining, dead code removal, copy propagation, constant folding, constant propagation, arithmetic optimizations , etc.

Optimizing shaders results in a higher FPS which can improve your gaming experience. We are using https://github.com/aras-p/glsl-optimizer which only supports OpenGL 1.4 (and not all STK shaders) so try to help update that project

It improved my FPS by **+10** on a Radeon HD 7640G.

## Optimizing shaders in STK on Linux
Run `tools/optimize_shaders.sh` in the STK directory and it will optimize most of the shaders.Ignore any errors in the ouput since cannot optimize all shaders it gives errors for them

## Optimizing shaders in STK on Windows 10 with Windows Linux Subsystem
NOTE : This method is untested
Microsoft's latest build of Windows 10 contains an Ubuntu subsystem which can run command line applications.Try running `tools/optimize_shaders.sh` in the STK directory using the Ubuntu subsystem and report any errors you get
