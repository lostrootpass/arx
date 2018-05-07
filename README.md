# Arx Fatalis


[Video](https://vimeo.com/pmurdoch/arx)

## Overview
---

Changes from the original Arx Fatalis:
* Programmable pipeline in OpenGL alongside the original fixed-function D3D7 pipeline.
* Reduced CPU load for animation by having the GPU do vertex manipulation instead.
* Per-pixel lighting in OpenGL alongside D3D7's per-vertex lighting model, allowing significant reduction in poly count in some areas (e.g. reductions of several thousand polys in cinematics).
* Shader-based particle and screen effects allowing for more efficient particle rendering (e.g. batching)
* Reduce framerate timing dependencies for key actions like movement
* GPU text rendering for OpenGL to supercede the original GDI/Win32 text model.
* SDL for input and window management
* Cleaned up some warnings (though many still remain)
* Miscellaneous tweaks and improvements


## Building
---

Currently, the game builds with many warnings that are a holdover from the legacy Arx codebase, so warnings-as-errors must be disabled. Warnings are currently being chipped away at over time, but there are a lot of them.

Dependencies for building under Visual Studio 2017 (msvc15):
* [August 2007 DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=13287)
* [SDL 2.0.5](http://libsdl.org)
* [glm](https://glm.g-truc.net/0.9.8/index.html)
* [stb headers](https://github.com/nothings/stb): stb\_image.h, stb\_truetype.h, stb\_rect\_pack.h
* [glew](http://glew.sourceforge.net/)

## License
---

The original GPL source on which this project was based can be found on the developer's website at [arkane-studios.com](http://arkane-studios.com).

Out of obligation to the original GPLv3 license that the Arx Fatalis source code was released with, this project has also been released under GPLv3, the full text of which can be found included with the project [here](ARX_PUBLIC_LICENSE.txt). 
