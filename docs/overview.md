# IntenseLogic Overview

IntenseLogic's design is somewhat modularised, which makes an overview easier.

## Project Layout
    IntenseLogic/   Project root (entries ending with / are directories, others are files)
    |-build/        This is where scons puts binaries
    | `-docs/       LDoc puts generated files here
    |-docs/         Non-specific IntenseLogic documentation, like this file
    |-script/       Lua API and related facilities; Modules here map 1:1 with the actual engine modules
    |-test/         Unit tests
    |-support/      Scripts used during the build process
    |-examples/     Examples of how to use the engine
    |-shaders/      GLSL shaders
    |-SConstruct    Controls the build process
    `-src           The actual engine
      |-common/     Code shared between other modules
      |-util/       Datastructures, logging, etc.
      |-math/       Math related to 3D graphics (vectors, matrices, quaternions)
      |-graphics/   Code that manages rendering
      | |-api/      Wrapper layer for some extensions and for some OpenGL functionality not present in early versions
      | `-sg/       Scene graph layer
      |-network/    Networking
      |-asset/      Filesystem management and file format loaders
      |-script/     Loads Lua scripts
      |-windows.c   Windows-specific code (mostly for functions from POSIX that MinGW doesn't implement)
      `-il.docopt   Docopt file, for command-line options


