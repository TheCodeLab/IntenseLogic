import os
#import support.docopt_c
import string

# defs
output    = "il"
src_dir   = "#src"
cli_file  = "src/il.docopt"
inputs    = "*.c script/*.c"
platform  = ARGUMENTS.get("platform", "linux")

# flags
ccflags   = "-Wall -Wextra -pedantic"
cflags    = "-std=c99 -D_POSIX_C_SOURCE=200809"
cxxflags  = "-std=c++11 -isystem-prefixbullet"
linkflags = "-L."
if platform == "mingw":
    cflags += " -DWIN32 -I/usr/x86_64-w64-mingw32/include/luajit-2.1 " # TODO: Fix this
    linkflags += " -Wl,--export-all-symbols"
else:
    linkflags += " -rdynamic" # assume ELF because I'm terrible

# build path
build_dir_raw = ARGUMENTS.get("build_dir", "build")
build_dir = "#"+build_dir_raw

#link flags
if platform=="osx":
    linkflags += " -pagezero_size 10000 -image_base 100000000"
linkflags += " -L"+build_dir_raw

# clang-specific flags
if "ASAN" in os.environ and "1" in os.environ["ASAN"]:
    ccflags   += " -fsanitize=address,undefined"
    linkflags += " -fsanitize=address,undefined"

# debug mode
build_mode = ARGUMENTS.get("mode", "debug")
if build_mode != "release":
    ccflags   += " -g -O0"
    linkflags += " -g"

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
env = Environment(ENV=os.environ)
for item in os.environ:
    env[item] = os.environ[item]
env.Append(CCFLAGS=" ", CFLAGS=" ", CXXFLAGS=" ", LINKFLAGS=" ") # Make CFLAGS/etc in env work right.
env.Append(CCFLAGS=ccflags, CFLAGS = cflags, CXXFLAGS=cxxflags, LINKFLAGS = linkflags, CPPPATH = [src_dir])

Export("platform")
Export("env")
Export("build_dir")
libilutil = SConscript("src/util/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/util")
Export("libilutil")
libilmath = SConscript("src/math/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/math")
Export("libilmath")
libilcommon = SConscript("src/common/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/common")
Export("libilcommon")
libilasset = SConscript("src/asset/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/asset")
Export("libilasset")
libilinput = SConscript("src/input/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/input")
Export("libilinput")
libilgraphics = SConscript("src/graphics/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/graphics")
Export("libilgraphics")
libilnetwork = SConscript("src/network/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/network")
Export("libilnetwork")
libillua = SConscript("src/lua/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/lua")
Export("libillua")
SConscript("test/math/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/test/math")
SConscript("demos/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/demos")
prog = SConscript("src/SConscript", env=env, duplicate = 0, variant_dir=build_dir+"/obj/prog")

Depends(prog, libilmath)
Depends(prog, libilnetwork)
Depends(prog, libilutil)
Depends(prog, libilcommon)
Depends(prog, libilgraphics)
Depends(prog, libilasset)
Depends(prog, libilinput)
Depends(prog, libillua)
#Depends(prog, libilphysics)
Default(prog)

