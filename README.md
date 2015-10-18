# IntenseLogic
IntenseLogic is a library for developing a game engine written in C99,
which runs on Linux, Windows, and OS X.

## Features

- 3D graphics primitives for rendering scenes with OpenGL 3.2
- Thread safe
- Deferred shading, HDR, MSAA, bloom
- Builtin primitives: heightmaps, wireframes, skyboxes, boxes,
  icosahedrons
- Never steals control flow
- PNG and OBJ importers
- Logging system with multiple sinks

## Demos

Demos can be found at https://github.com/TheCodeLab/IntenseLogicDemos

![Bouncing Lights Demo](http://i.imgur.com/JhcLYPO.png)

### Dependencies

- OpenGL 3.2
- [tiffgl](https://github.com/tiffany352/tiffgl)
- [libepoxy](https://github.com/anholt/libepoxy) (default) or [GLEW](http://glew.sourceforge.net/) (requires a special flag)
- [libpng](http://libpng.org/)

### Building

Build files for tup are provided.

Manual builds must satisfy the following:

- The 'src' directory must be on the include path
- Each dependency must be on the include path
- TiffGL requires you to decide whether you are using libepoxy or
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
