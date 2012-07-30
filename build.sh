if (test -e CC); then
CC="$CC";
else
CC=gcc;
fi;

echo "Compiler: $CC";

INCLUDES="$INCLUDES -I. -I../include"
CFLAGS="$CFLAGS -Wall -pg -DdDOUBLE $INCLUDES"
CFLAGS="$CFLAGS `sdl-config --cflags`"
LDFLAGS="$LDFLAGS -Llib/ -lSOIL -lm -pg $INCLUDES"
#LDFLAGS="$LDFLAGS `sdl-config --libs`"

echo "INCLUDES: $INCLUDES";
echo "CFLAGS: $CFLAGS";
echo "LDFLAGS: $LDFLAGS";

if $(test $1 = "mingw"); then
EXTENSION=.exe
LINKSUFFIX=.dll
LDFLAGS="$LDFLAGS -static-libgcc -static -llua -lopengl32"
echo "Target: mingw";
else
LINKSUFFIX=.so
LDFLAGS="$LDFLAGS -llua5.1 -lc -lGL -lSDL"
echo "Target: linux";
fi;

cd src;

SOURCES="main.c common/*.c graphics/*.c network/*.c script/*.c asset/*.c" # physics/*.c

echo "SOURCES: $SOURCES";

for f in $SOURCES; do
echo "$CC $CFLAGS -c $f -o ../obj/$(basename $f .c).o";
$CC $CFLAGS -c $f -o ../obj/$(basename $f .c).o;
done;

cd ..

echo "$CC obj/*.o lib/*$LINKSUFFIX $LDFLAGS -o bin/il$EXTENSION"
$CC obj/*.o lib/*$LINKSUFFIX $LDFLAGS -o bin/il$EXTENSION
