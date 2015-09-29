# IntenseLogic
IntenseLogic is a library for developing a game engine written in C99, which runs on Linux, Windows, and OS X.

## Components

### Graphics
A 3D graphics library for rendering scenes using OpenGL, which can run in its own thread.

Supports deferred shading, HDR, MSAA.
Can be extended with custom renderer types and custom coordinate systems
(Only single precision floating point is provided but it is not hard to implement double precision or fixed point.)

### Asset
A VFS layer with importers for PNG and OBJ formats.
Uses mmap or MapViewOfFile to map files into memory.

### Util
- Data structures: vectors, linked lists, hash tables from uthash
- Third party code: uthash
- Plugin loader
- Event hook system
- Command line option parsing
- Logging system with multiple sinks

### Math
Contains useful 3D graphics math: 3d/4d vectors, matrices, and quaternions.

## Demos

Demos can be found at https://github.com/TheCodeLab/IntenseLogicDemos

![Bouncing Lights Demo](http://i.imgur.com/JhcLYPO.png)

### Dependencies

- OpenGL 3.2
- [tiffgl](https://github.com/tiffany352/tiffgl)
- [SDL 2.0](http://libsdl.org/)
- [libepoxy](https://github.com/anholt/libepoxy) (default) or [GLEW](http://glew.sourceforge.net/) (requires a special flag)
- [libpng](http://libpng.org/)
- pthread

### Building

Build files for tup are provided.

Manual builds must satisfy the following:

- The 'src' directory must be on the include path
- Each dependency must be on the include path
- Tiffgl requires you to decide whether you are using libepoxy or
  GLFW through a compiler define
- It is preferred to create statically linked libraries for IL, as it
  is not ABI or API compatible between versions. You should vendor
  this repository as a submodule.
- Code should be compiled as C with -std=gnu99.

## Contact

### IRC

[Web Chat](https://webchat.freenode.net/?channels=##codelab)

    Host: irc.freenode.net
    Channel: ##codelab

### Twitter

[`@tiffany_public`](https://twitter.com/tiffany_public)

## License
Copyright (c) 2012 IntenseLogic Development Team

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
