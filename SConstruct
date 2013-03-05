import os
import support.docopt_c

# defs
output    = "il"
src_dir   = "#src"
build_dir = "build"
cli_file  = "src/il.docopt"
inputs    = "*.c common/*.c graphics/*.c network/*.c script/*.c asset/*.c"
platform  = ARGUMENTS.get("platform", "linux")

# flags
cflags    = "-Wall -Wextra -pedantic -std=c99 -g -DDEBUG"
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
    "osx":   ["ilmath", "m", "png"],
    "mingw": ["ilmath", "mingw32", "libevent", "ws2_32", "glfw", "glew32", "opengl32", "png", "z", "lua51"],
    "arch":  ["ilmath", "m", "png"],
    "linux": ["ilmath", "m", "png"]
}

pkg_libs = {
    "osx":   ["libevent", "libglfw", "glew", "luajit"],
    "arch":  ["libevent", "libglfw", "glew", "luajit", "gl"],
    "linux": ["libevent", "gl", "glfw", "glew", "luajit"],
    "mingw": []
}

# link libs
VariantDir(build_dir, src_dir, duplicate = 0)
env = Environment(CCFLAGS = cflags, LINKFLAGS = linkflags)

env['CPPPATH'] = [src_dir]

for lib in pkg_libs[platform] :
    env.ParseConfig("pkg-config " + lib + " --cflags --libs")

Export("platform")
Export("env")
libilmath = SConscript("src/math/SConscript", platform=platform)
SConscript("test/SConscript", platform=platform)

#for lib in libs[platform] :
env.Append(LIBS = libs[platform])

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

# link program
prog = env.Program(target  = build_dir + "/" + output,
                   source  = objects,
                   LIBPATH = lib_dirs,
                   CPPPATH = src_dir)
Depends(prog, libilmath)
Default(prog)

