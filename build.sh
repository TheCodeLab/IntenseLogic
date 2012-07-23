if (test -e CC); then
CC="$CC";
else
CC=gcc;
fi;

echo "Compiler: $CC";

INCLUDES="$INCLUDES -I."
CFLAGS="$CFLAGS -Wall -g -DdDOUBLE $INCLUDES"
LDFLAGS="$LDFLAGS -lm -lode $INCLUDES"

echo "INCLUDES: $INCLUDES";
echo "CFLAGS: $CFLAGS";
echo "LDFLAGS: $LDFLAGS";

if $(test $1 = "mingw"); then
EXTENSION=.exe
LDFLAGS=$LDFLAGS -static -llua
echo "Target: mingw";
else
LDFLAGS=$LDFLAGS -llua5.1 -lc -lGL
echo "Target: linux";
fi;

cd src;

SOURCES="main.c common/*.c physics/*.c graphics/*.c network/*.c script/*.c"

echo "SOURCES: $SOURCES";

for f in $SOURCES; do
$CC $CFLAGS -c $f -o ../obj/$(basename $f .c).o;
done;

cd ..

$CC $LDFLAGS obj/*.o -o bin/il$EXTENSION
