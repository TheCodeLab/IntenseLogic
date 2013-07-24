import os
#import support.docopt_c
import string
import subprocess

# defs
output    = "il"
src_dir   = "#src"
build_dir = "build"
cli_file  = "src/il.docopt"
inputs    = "*.c script/*.c"
platform  = ARGUMENTS.get("platform", "linux")

# flags
cflags    = "-Wall -Wextra -pedantic -std=c99 -g -O0 -DDEBUG -D_POSIX_C_SOURCE=200809"
linkflags = "-g -L. -Lbuild"
if platform == "mingw":
    cflags += " -DWIN32 -I/usr/i486-mingw32/include/luajit-2.0" # TODO: get rid of that monstrosity
    linkflags += " -Wl,--export-all-symbols"
else:
    linkflags += " -rdynamic" # assume ELF because I'm terrible

# libs
lib_dirs = ["/usr/lib", "/usr/local/lib"]

libs = {
    "osx":   ["dl"],
    "mingw": ["mingw32", "lua51"],
    "arch":  ["dl"],
    "linux": ["dl"]
}

pkg_libs = {
    "osx":   ["luajit"],
    "arch":  ["luajit"],
    "linux": ["luajit"],
    "mingw": []
}

# link libs
VariantDir(build_dir, src_dir, duplicate = 0)
env = Environment(CCFLAGS = cflags, LINKFLAGS = string.split(linkflags, " "))
if 'CC' in os.environ:
    env['CC'] = os.environ['CC']
if 'LD' in os.environ:
    env['LD'] = os.environ['LD']
if 'SHLIBSUFFIX' in os.environ:
    env['SHLIBSUFFIX'] = os.environ['SHLIBSUFFIX']
if 'PROGSUFFIX' in os.environ:
    env['PROGSUFFIX'] = os.environ['PROGSUFFIX']

env.Append(CPPPATH = [src_dir])

Export("platform")
Export("env")
libilutil = SConscript("src/util/SConscript", platform=platform, env=env)
Export("libilutil")
libilmath = SConscript("src/math/SConscript", platform=platform, env=env)
Export("libilmath")
libilcommon = SConscript("src/common/SConscript", platform=platform, env=env, libilutil=libilutil, libilmath=libilmath)
Export("libilcommon")
libilasset = SConscript("src/asset/SConscript", platform=platform, env=env, libilutil=libilutil, libilmath=libilmath, libilcommon=libilcommon)
Export("libilasset")
libilinput = SConscript("src/input/SConscript", platform=platform, env=env, libilutil=libilutil, libilcommon=libilcommon)
Export("libilinput")
libilgraphics = SConscript("src/graphics/SConscript", platform=platform, env=env, libilutil=libilutil, libilmath=libilmath, libilcommon=libilcommon, libilasset=libilasset, libilinput=libilinput)
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
#handle = open("src/docopt.inc", "w")
#handle.write(support.docopt_c.output(cli_file))
#handle.close()

# generate version information
handle = open("src/version.h", "w")
handle.write("// This file was generated by SConstruct. Do not modify. Create a tag if you wish to increment the version.\n\n")
handle.write("static const char *il_version = \"")
handle.write(subprocess.check_output(["git", "describe", "--dirty", "--always", "--tags"]).rstrip())
handle.write("\";\n")
handle.close()

# generate object files
objects = env.Object(source = sources)

#env.Append(LINKFLAGS=["-lilutil"])
env.Append(LIBS = libs[platform])
for lib in pkg_libs[platform] :
    env.ParseConfig("pkg-config " + lib + " --cflags --libs")

lf=""
if platform=="osx":
    lf = "-pagezero_size 10000 -image_base 100000000"

# link program
prog = env.Program(target  = build_dir + "/" + output,
                   source  = objects,
                   LIBPATH = lib_dirs,
                   CPPPATH = src_dir,
                   LINKFLAGS = lf)
Depends(prog, libilmath)
Depends(prog, libilnetwork)
Depends(prog, libilutil)
Depends(prog, libilcommon)
Depends(prog, libilgraphics)
Depends(prog, libilasset)
Depends(prog, libilinput)
Default(prog)

