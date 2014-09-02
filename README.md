# IntenseLogic
IntenseLogic is a set of libraries for developing a game or game engine written in C99, which runs on Linux, Windows, and OS X.

## Libraries

### Graphics
A 3D graphics library for rendering scenes using OpenGL, which can run in its own thread.

Supports deferred shading, HDR, MSAA, and has very good performance as well as a model which maps well to the GPU.
Can be extended with custom renderer types and custom coordinate systems
(Only single precision floating point is provided but it is not hard to implement double precision or fixed point.)

### Asset
A VFS layer with importers for PNG and OBJ formats.
Uses mmap or MapViewOfFile to map files into memory, rather than copying them a la stdio.

### Util
- Data structures: vectors, linked lists, hash tables from uthash
- Third party code: uthash
- Plugin loader
- Event hook system
- Command line option parsing
- Logging system with multiple sinks

### Math
Contains useful 3D graphics math: 3d/4d vectors, matrices, and quaternions.

## Screenshots

![Bouncing Lights Demo](http://i.imgur.com/JhcLYPO.png)

## Documentation

(very out of date)

- [C API Documentation](https://lymiahugs.com/~tiffany/il/api/index.html)

## Building
Build system based on tup [Tup](http://gittup.org/tup/).

### Dependencies

- OpenGL 3.1
- [tiffgl](https://github.com/tiffany352/tiffgl) (provided as submodule)
- [SDL 2.0](http://libsdl.org/)
- [Libepoxy](https://github.com/anholt/libepoxy) (default) or [GLEW](http://glew.sourceforge.net/) (requires a special flag)
- [libpng](http://libpng.org/)
- pthread

### Building

    git submodule update --init --recursive
    git pull --recurse-submodules
    tup init
    tup

See the Wiki for full and detailed [build instructions](https://github.com/TheCodeLab/IntenseLogic/wiki/Building).

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
