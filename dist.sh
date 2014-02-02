#!/bin/bash

error() {
  echo "!!! $1"
  exit 1
}

version=$(git describe --dirty --always --tags)
_32bit() {
  export CFLAGS="-m32"
  export LINKFLAGS="-m32"
  export mingw_pre="i586"
}
_64bit() {
  export CFLAGS="-m64"
  export LINKFLAGS="-m64"
  export mingw_pre="x86_64-w64"
}
linux() {
  export platform="linux"
  export dysuffix=".so"
  export binsuffix=""

  export CC=clang
  export CXX=clang++
}
windows() {
  export platform="mingw"
  export dysuffix=".dll"
  export binsuffix=".exe"

  export CC="$mingw_pre-mingw32msvc-gcc"
  export CXX="$mingw_pre-mingw32msvc-g++"
}
case "$1" in
  "linux-32") _32bit; linux   ;;
  "linux-64") _64bit; linux   ;;
  "win-32"  ) _32bit; windows ;;
  "win-64"  ) _64bit; windows ;;
  *)
    echo "!!! Unknown platform $1"
    exit 1
    ;;
esac

build_dir=build_$1
dist_dir=dist/intenselogic_$1_$version

scons platform=$platform build_dir=$build_dir . || error "Build failed!"

rm -rf $dist_dir
mkdir -p $dist_dir

cp README.md $dist_dir
mkdir $dist_dir/bin
cp $build_dir/il$binsuffix $dist_dir/bin
mkdir $dist_dir/plugins
cp $build_dir/*$dysuffix $dist_dir/plugins
cp -r demos script shaders $dist_dir
cp -r distfiles/$platform/* $dist_dir
zip $dist_dir.zip $(find $dist_dir)
