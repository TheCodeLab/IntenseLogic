# IntenseLogic Overview

IntenseLogic is a highly modular project. At first, it was a monolithic game engine, but over time it has matured into something closer to a framework or set of libraries. The main binary is only a handful of files, and it loads plugins and executes scripts. Everything else is implemented through plugins.

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
      | `-gui/      GUI rendering
      |-network/    Networking
      |-asset/      Filesystem management and file format loaders
      |-script/     Loads Lua scripts
      `-windows.c   Windows-specific code (mostly for functions from POSIX that MinGW doesn't implement)

## Engine Diagram

 * Ovals represent abstractions, layers, or libraries
 * Boxes represent modules
 * Dotted lines represent dynamically loaded libraries, through dlsym or equivalent

![Engine Diagram](http://i.imgur.com/g7KWQaI.png)

