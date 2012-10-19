#!/bin/bash

TARGET=$1

if $(test "$TARGET" = "mingw"); then
	EXECUTABLE="bin/il.exe";
else
	EXECUTABLE="bin/il";
fi;

VERSION="1.0-prealpha"
BUILDPATH="intenselogic-$TARGET-$VERSION"
LIBURL="http://tiffany.stormbit.net/il/$TARGET-$VERSION.tar.gz"

echo "Packaging $BUILDPATH.tar.gz for $TARGET"
echo "Executable: $EXECUTABLE"

if $(test -e "$BUILDPATH" && ! test -d "$BUILDPATH"); then
	echo "$BUILDPATH is not a directory";
	rm -rf $BUILDPATH; # ??? not a directory
fi;

if $(test -d "$BUILDPATH"); then
	echo "Cleaning up";
	rm -rf $BUILDPATH;
fi;

mkdir $BUILDPATH;

if $(test ! -f $EXECUTABLE); then
	echo "No binary found, building";
	./build.sh $TARGET;
fi;

echo "Downloading shared libraries for dist"
rm $TARGET-$VERSION.tar.gz
wget $LIBURL
echo "Extracting shared libraries"
tar -xzf $TARGET-$VERSION.tar.gz # extracts into $BUILDPATH folder

echo "Copying binary"
cp $EXECUTABLE $BUILDPATH

echo "Copying README"
cp README.md $BUILDPATH/README.md

echo "Packaging"
rm intenselogic-$TARGET-$VERSION.tar.gz
tar -czf intenselogic-$TARGET-$VERSION.tar.gz $BUILDPATH

