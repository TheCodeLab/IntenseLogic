if (test -e CC); then
CC="$CC";
else
CC=gcc;
fi;

echo "Compiler: $CC";

INCLUDES="$INCLUDES -I."
CFLAGS="$CFLAGS -Wall -g -DdDOUBLE $INCLUDES"
LDFLAGS="$LDFLAGS -Llib/ -lm $INCLUDES"

echo "INCLUDES: $INCLUDES";
echo "CFLAGS: $CFLAGS";
echo "LDFLAGS: $LDFLAGS";

if $(test $1 = "mingw"); then
EXTENSION=.exe
LDFLAGS="$LDFLAGS -static-libgcc -static-libstdc++ -static -llua -lode"
echo "Target: mingw";
else
LDFLAGS="$LDFLAGS -llua5.1 -lc -lGL -lode"
echo "Target: linux";
fi;

cd src;

SOURCES="main.c common/*.c graphics/*.c network/*.c script/*.c" # physics/*.c

echo "SOURCES: $SOURCES";

for f in $SOURCES; do
$CC $CFLAGS -c $f -o ../obj/$(basename $f .c).o;
done;

cd ..

$CC obj/*.o $LDFLAGS -o bin/il$EXTENSION
