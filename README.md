# IntenseLogic
IntenseLogic is a set of libraries for developing a game or game engine written in C99, which runs on Linux, Windows, and OS X.

## Libraries

### Graphics
A 3D graphics library for rendering scenes using OpenGL, which runs on its own thread.

Supports deferred shading and custom renderers.

Contains simple GUI support, including a fully featured text renderer using Cairo, HarfBuzz, and FreeType2. It is capable of correctly rendering almost any language, provided the loaded font supports it.

### Asset
A VFS layer with importers for PNG and OBJ formats.

### Common
Contains some stuff that the other libraries depend on, such as a container for objects placed in the world.

### Util
Some generally useful stuff (logging, uthash, macros)

### Math
Contains all that stuff useful for 3D graphics: vectors (3/4d), matrices, quaternions

## Screenshots

![Bouncing Lights Demo](http://i.imgur.com/CXaIVHY.png)

## Documentation
(very out of date)

- [C API Documentation](https://lymiahugs.com/~tiffany/il/api/index.html)
- [Lua API Documentation](https://lymiahugs.com/~tiffany/il/doc/index.html)

## Building
Compile using [SCons](http://scons.org/).

List of Dependencies:

- OpenGL 3.1
- [SDL 2.0](http://libsdl.org/)
- [GLEW](http://glew.sourceforge.net/)
- [libevent2](http://libevent.org)
- [libpng](http://libpng.org/)
- [LuaJIT (Lua 5.1)](http://luajit.org/)
- [HarfBuzz (with ICU)](http://www.freedesktop.org/wiki/Software/HarfBuzz/)
- [Cairo](http://cairographics.org/)
- [FreeType2](http://freetype.org/)
- pthread

See the Wiki for help [building](https://github.com/TheCodeLab/IntenseLogic/wiki/Building).

## Contact

### IRC

[Web Chat](https://webchat.freenode.net/?channels=##codelab)

    Host: irc.freenode.net
    Channel: ##codelab

### Twitter

[`@tiffany_public`](https://twitter.com/tiffany_public)

## License
Copyright (c) 2012 Intense Logic Development Team

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

