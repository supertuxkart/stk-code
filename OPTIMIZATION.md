# Optimizing SuperTuxKart shaders
Most shaders are unoptimized after they are written

A shader optmizer takes the unoptimized shaders code and does optimization like function inlining, dead code removal, copy propagation, constant folding, constant propagation, arithmetic optimizations and so on.

Optimizing shaders results in a higher FPS , less heat (and noise) and smoother gameplay.We are using https://github.com/aras-p/glsl-optimizer which only supports OpenGL 1.4 (and not all STK shaders) so try to help update that project

It improved my FPS by **+10** on a Radeon HD 7640G.

## Optimizing shaders in STK on Linux
To optimize your shaders just run `tools/optimize_shaders.sh` in the STK directory and it will optimize most of the shaders.Ignore any errors in the ouput since cannot optimize all shaders it gives errors for them

## Optimizing shaders in STK on Windows 10 with Windows Linux Subsystem
NOTE : This method is untested
Microsoft's latest build of Windows 10 contains an Ubuntu subsystem (yes , Ubuntu subsystem) which can run command line applications.Try running `tools/optimize_shaders.sh` in the STK directory using the Ubuntu subsystem
