CC=gcc;

echo "Compiler: $CC";

INCLUDES="$INCLUDES -I."
CFLAGS="$CFLAGS -Wall -Wextra -g -DDEBUG $INCLUDES"
CFLAGS="$CFLAGS `sdl-config --cflags`"
LDFLAGS="$LDFLAGS -Llib/ -levent_core -levent_extra -lm -g $INCLUDES"
#LDFLAGS="$LDFLAGS `sdl-config --libs`"

echo "INCLUDES: $INCLUDES";
echo "CFLAGS: $CFLAGS";
echo "LDFLAGS: $LDFLAGS";

if $(test "$1" = "mingw"); then
EXTENSION=.exe
LINKSUFFIX=.dll
CFLAGS="$CFLAGS -I../include"
LDFLAGS="-lmingw32 -lSDLmain -lSDL $LDFLAGS -lws2_32 -static-libgcc -static -llua -lopengl32 -lglew32"
echo "Target: mingw";
else
LINKSUFFIX=.so
LDFLAGS="$LDFLAGS -llua -lc -lGL -lSDL -lGLEW"
echo "Target: linux";
fi;

cd src;

SOURCES="*.c common/*.c graphics/*.c network/*.c script/*.c asset/*.c" # physics/*.c

echo "SOURCES: $SOURCES";

for f in $SOURCES; do
echo "$CC $CFLAGS -c $f -o ../obj/$(basename $f .c).o";
$CC $CFLAGS -c $f -o ../obj/$(basename $f .c).o;
done;

cd ..

echo "$CC obj/*.o $LDFLAGS -o bin/il$EXTENSION"
$CC obj/*.o $LDFLAGS -o bin/il$EXTENSION

