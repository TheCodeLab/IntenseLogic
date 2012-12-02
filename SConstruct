import os
import support.docopt_c

# defs
output     = "il"
src_dir    = "src"
build_dir  = "build"
cli_file   = "src/il.docopt"
compiler   = "clang"

libs       = ["m"]
pkg_libs   = ["libevent", "glew", "lua", "libglfw"]
lib_dirs   = ["/usr/lib", "/usr/local/lib"]

mingw_libs = ["mingw32", "ws2_32", "opengl32"]

inputs     = "*.c common/*.c graphics/*.c network/*.c script/*.c asset/*.c"

# flags
cflags    = "-Wall -g -DDEBUG -I./" + src_dir
linkflags = "-g"

# libs
env = Environment(CCFLAGS = cflags, LINKFLAGS = linkflags, CC = compiler)

for lib in pkg_libs :
    env.ParseConfig("pkg-config " + lib + " --cflags --libs")

for lib in libs :
    env.Append(LIBS = lib)

if env["PLATFORM"] == "mingw" :
    for lib in mingw_libs :
        env.Append(LIBS = lib)

# get sources
sources = []
for module in Split(inputs) :
    sources.extend(Glob(src_dir + "/" + module))

# generate docopt
handle = open(src_dir + "/docopt.inc", "w")
handle.write(support.docopt_c.output(cli_file))
handle.close()

# generate object files
objects = env.Object(source = sources)

# link program
VariantDir(build_dir, src_dir)

env.Program(target  = build_dir + "/" + output,
            source  = objects,
            LIBPATH = lib_dirs)
