# Plugin Writing Tutorial

This tutorial will introduce you to the module system of IntenseLogic, as well as aid you in setting up your basic development environment.

## Development Environment

You don't have to set this up precisely as I describe it, but a few things are important:

- That you have a directory that only .so's you intend to be loaded as plugins are in
- That you have a directory with all of the lua scripts you wish to use are located
- That you have a way to do test runs without having to copy files around

The first step you'll want to do is to setup a new git repository, and get a simple file structure going.

    mkdir my_plugin && cd my_plugin
    git init
    git submodule add https://github.com/TheCodeLab/IntenseLogic.git
    mkdir build     # Output directory of your build system, where the .so goes
    mkdir src       # Where the code goes
    mkdir script    # Where the Lua bindings will go
    mkdir shaders   # If your plugin uses any shaders
    mkdir assets    # Any miscellaneous files that you'll need to include with a distribution of your plugin

You should now setup your build system and compile the copy of IL. The next thing we'll do is write a shell script to invoke the engine and tell it to load your plugin:

    #!/bin/bash

    LD_LIBRARY_PATH=build:IntenseLogic/build IntenseLogic/build/il -m build -m IntenseLogic/build --scripts script --scripts IntenseLogic/script --shaders IntenseLogic/shaders

    # Append --shaders shaders/ if you have new shaders
    # Use the -r if you wish to execute a script on startup

We're ready to start writing a simple plugin.

## The Code

You might be wondering what kind of things modules actually do, this is where I'll explain it. A module in IntenseLogic is essentially a miniature program that doubles as a library. It can be linked to, but it also has its own main() (called il_bootstrap) and can run code of its own. Modules can link to other modules, which is why the script to start the engine requires that LD_LIBRARY_PATH variable. If you make a game written in C instead of Lua, you'll be implementing it as a plugin which calls other modules the same way the Lua code would.

It is generally a good idea to keep your plugins towards the library side and have Lua calls to set them up, which allows run-time tinkering. This way, you won't have to recompile if you want to add a teapot model to the scene or something.

The lifetime of the engine goes something like this:

1. The main binary fires up and parses command line options
2. The main binary starts loading plugins from the provided directories
3. Each plugin is dlopen/LoadLibrary()'d, gets dependencies checked (via il_dependencies()), and if it has any, the engine loads those first.
4. Each plugin's il_bootstrap() is called with argc and argv, allowing them to setup their environments
5. All scripts are executed
6. Main loop
7. Shutdown event

As you might have guessed, the most basic plugin you can write looks something like this:

    const char **il_dependencies(int argc, char **argv)
    {
        (void) argc, (void)argv;
        const char *dependencies[] = {
            "ilgraphics",
            "foobarbazz"
        };
        return dependencies;
    }

    int il_bootstrap(int argc, char **argv)
    {
        // do something
    }

## Distribution

If you've done everything right, it should be very easy to distribute your plugin. You should be able to compile for Windows, Linux, and OS X. Each OS's shared object, the scripts, and any other resources, should be packaged into a .zip, and you can distribute it. Users should be able to simply extract the zip over the current directory and everything will go into the right locations. The engine will only try to load shared objects that are of the right format, so it is okay to have a mix of .so, .dylib, and .dll in the plugins directory. In the future, users won't even need to unzip the plugin to use it, they will simply drop it into the directory IL looks for plugins.

## Suggestions

Please don't write plugins in C simply to avoid people from looking at the source. If they so desired, they could simply use a decent decompiler anyway. And don't make windows-only plugins, either. Many users would be unhappy with you for doing that, including the main developer of the engine and writer of this document (tiffany). It's not that hard to do cross-platform development, it's only hard to port from code written in heavy Win32 and Direct X. 

Try to follow the rest of the engine's coding practices of creating as few singletons and mutable global variables as possible, because you almost always end up realizing you (or your users) need more than one of an object eventually. IntenseLogic has very few global variables, and most of those that do exist are either planned to be fixed or are declared static. 

If you wish your adventures with Lua bindings to go well, I heavily suggest against reference-counting or other kinds of garbage collection. Objects should be owned by C code, cleaned up by C code, and only exposed to Lua through functions and integer/opaque pointer handles. I've waded through this before, and it is not a fun path to tread. Memory leaks and use-after-frees are a given when you have manually reference counted C on one hand, and Lua-managed memory on the other. It's much easier to handle serialization and other introspective tasks when great care is taken to keep your objects isolated. It is good for cache performance when you enforce locality by putting objects into arrays, which is easy when you use integer handles. 

Creating threads should only be done if there is no reliable mechanism for asynchronous access, such as the fact that SwapInterval() is ignored depending on the driver, and that there is no reliable way to do async disk IO. Any kind of purely computational tasks should be scheduled with the event mechanism and great care should be taken to avoid threading bugs.

