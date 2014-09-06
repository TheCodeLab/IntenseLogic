# IntenseLogic Overview

IntenseLogic is a set of libraries used to create a game engine.

## Project Layout

    IntenseLogic/       Project root (entries ending with / are directories, others are files)
    |-docs/             Non-specific IntenseLogic documentation, like this file
    |-test/             Unit tests
    |-examples/         Examples of how to use the engine
    | `-main.c[pp]      Examples on how to set up for a game (command line parsing + initialization)
    |-demos/            Full demos which can be compiled and run, and are built by default
    | |-bouncing-lights Spawns one hundred lights which are rigid bodies simulated by Bullet
    | |-teapots/        Shows a teapot with the camera rotating around it, with lighting
    | |-box/            Shows a rainbow-coloured box with the camera rotating around it
    | `-quad/           Renders a fullscreen quad, as a sanity test
    |-shaders/          GLSL shaders
    |-Tupfile           For building
    `-src/              The actual engine
      |-util/           Events, datastructures, logging, third party code (uthash), etc.
      |-math/           Math related to 3D graphics (vectors, matrices, quaternions)
      |-graphics/       Code that manages rendering
      `-asset/          Filesystem management and file format loaders
