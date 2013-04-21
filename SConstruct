import os
import support.docopt_c
import string

# defs
output    = "il"
src_dir   = "#src"
build_dir = "build"
cli_file  = "src/il.docopt"
inputs    = "*.c script/*.c asset/*.c"
platform  = ARGUMENTS.get("platform", "linux")

# flags
cflags    = "-Wall -pedantic -std=c99 -g -O0 -DDEBUG -D_POSIX_C_SOURCE=200809"
linkflags = "-g -L. -Lbuild"
if platform == "mingw":
    cflags += " -DWIN32"
    linkflags += " -Wl,--export-all-symbols"
else:
    linkflags += " -rdynamic" # assume ELF because I'm terrible

# luajit requires this
if platform == "osx":
    linkflags += " -pagezero_size 10000 -image_base 100000000"

# libs
lib_dirs = ["/usr/lib", "/usr/local/lib"]

libs = {
    "osx":   ["m", "png"],
    "mingw": ["mingw32", "ws2_32", "glfw", "glew32", "opengl32", "png", "z", "lua51"],
    "arch":  ["m", "png"],
    "linux": ["m", "png"]
}

pkg_libs = {
    "osx":   ["libglfw", "glew", "luajit"],
    "arch":  ["libglfw", "glew", "luajit", "gl"],
    "linux": ["gl", "glfw", "glew", "luajit"],
    "mingw": []
}

# link libs
VariantDir(build_dir, src_dir, duplicate = 0)
env = Environment(CCFLAGS = cflags, LINKFLAGS = string.split(linkflags, " "))
if 'CC' in os.environ:
    env['CC'] = os.environ['CC']

env.Append(CPPPATH = [src_dir])

Export("platform")
Export("env")
libilutil = SConscript("src/util/SConscript", platform=platform, env=env)
Export("libilutil")
libilmath = SConscript("src/math/SConscript", platform=platform, env=env)
Export("libilmath")
libilcommon = SConscript("src/common/SConscript", platform=platform, env=env, libilutil=libilutil, libilmath=libilmath)
Export("libilcommon")
libilgraphics = SConscript("src/graphics/SConscript", platform=platform, env=env, libilutil=libilutil, libilmath=libilmath, libilcommon=libilcommon)
Export("libilgraphics")
libilnetwork = SConscript("src/network/SConscript", platform=platform, env=env, libilutil=libilutil)
Export("libilnetwork")
SConscript("test/SConscript", platform=platform, env=env)

# get sources
sources = []
for module in Split(inputs) :
    sources.extend(Glob(build_dir + "/" + module))

# generate docopt
#handle = open(src_dir + "/docopt.inc", "w")
handle = open("src/docopt.inc", "w")
handle.write(support.docopt_c.output(cli_file))
handle.close()

# generate object files
objects = env.Object(source = sources)

env.Append(LINKFLAGS=["-lilutil", "-lilmath", "-lilnetwork", "-lilcommon", "-lilgraphics"])
env.Append(LIBS = libs[platform])
for lib in pkg_libs[platform] :
    env.ParseConfig("pkg-config " + lib + " --cflags --libs")

# link program
prog = env.Program(target  = build_dir + "/" + output,
                   source  = objects,
                   LIBPATH = lib_dirs,
                   CPPPATH = src_dir)
Depends(prog, libilmath)
Depends(prog, libilnetwork)
Depends(prog, libilutil)
Depends(prog, libilcommon)
Depends(prog, libilgraphics)
Default(prog)

