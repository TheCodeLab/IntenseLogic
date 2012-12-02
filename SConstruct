import os
import support.docopt_c

# defs
output     = "il"
src_dir    = "src"
build_dir  = "build"
cli_file   = "src/il.docopt"
inputs     = "*.c common/*.c graphics/*.c network/*.c script/*.c asset/*.c"

# flags
cflags    = "-Wall -g -DDEBUG -I./" + src_dir
linkflags = "-g"

# libs
lib_dirs = ["/usr/lib", "/usr/local/lib"]

libs = {
    "osx": ["m", "lua"],
    "mingw": ["mingw32", "ws2_32", "glfw", "opengl32"],
    "arch": ["m"],
    "linux": ["m", "lua"]
}

pkg_libs = {
    "osx": ["libevent", "libglfw", "glew"],
    "arch": ["libevent", "libglfw", "glew", "lua"],
    "linux": ["libevent", "gl", "glfw", "glew"]
}

# link libs
VariantDir(build_dir, src_dir, duplicate = 0)
platform = ARGUMENTS.get("platform", "linux")
env      = Environment(CCFLAGS = cflags, LINKFLAGS = linkflags)

for lib in pkg_libs[platform] :
    env.ParseConfig("pkg-config " + lib + " --cflags --libs")

for lib in libs[platform] :
    env.Append(LIBS = lib)

# get sources
sources = []
for module in Split(inputs) :
    sources.extend(Glob(build_dir + "/" + module))

# generate docopt
handle = open(src_dir + "/docopt.inc", "w")
handle.write(support.docopt_c.output(cli_file))
handle.close()

# generate object files
objects = env.Object(source = sources)

# link program
env.Program(target  = build_dir + "/" + output,
            source  = objects,
            LIBPATH = lib_dirs)
