#!/bin/bash

error() {
  echo "!!! $1"
  exit 1
}

version=$(git describe --dirty --always --tags)
_32bit() {
  export CFLAGS="-m32"
  export LINKFLAGS="-m32"
}
_64bit() {
  export CFLAGS="-m64"
  export LINKFLAGS="-m64"
}
linux() {
  export platform="linux"
  export dysuffix=".so"
  export binsuffix=""
  export CFLAGS="-L/usr/lib/x86_64-linux-gnu "$CFLAGS
}
case "$1" in
  "linux-32")
    _32bit
    linux
    ;;
  "linux-64")
    _64bit
    linux
    ;;
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
