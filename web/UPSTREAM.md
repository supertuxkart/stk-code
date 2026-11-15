# Browser port attribution

The GLES/Vulkan guards, browser audio pumping, input limitations, and texture
compatibility changes adapt the experimental port by **ading2210**:

- Source: https://github.com/ading2210/stk-code/tree/wasm
- Reference revision: `43d3673a765d2bc2d01adc29fc87e990b7c90a3d`
- Port-only comparison base: the second parent of `332924c`.

These changes are adapted to this repository's `dev` revision rather than
replacing the game with the older fork. Existing source licenses and copyright
notices apply; see [COPYING](../COPYING).

The isolated CMake entry point, local build/asset tools, page, save lifecycle,
and documented milestone history are maintained in this repository.
