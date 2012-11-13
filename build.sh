#!/bin/bash

CC=${CC:-gcc};
LD=${LD:-gcc};

echo "Compiler: $CC";
echo "Linker: $LD";

INCLUDES="$INCLUDES -I."
CFLAGS="$CFLAGS -Wall -g -DDEBUG $INCLUDES" #-Wextra -pedantic
CFLAGS="$CFLAGS $(pkg-config --cflags libevent)"
CFLAGS="$CFLAGS $(pkg-config --cflags glew)"
CFLAGS="$CFLAGS $(pkg-config --cflags lua)"
LDFLAGS="$LDFLAGS $(pkg-config --libs libevent)"
LDFLAGS="$LDFLAGS $(pkg-config --libs glew)"
LDFLAGS="$LDFLAGS $(pkg-config --libs lua)"
LDFLAGS="$LDFLAGS -lm -g";

echo "INCLUDES: $INCLUDES";
echo "CFLAGS: $CFLAGS";
echo "LDFLAGS: $LDFLAGS";

if $(test "$1" = "mingw"); then
	EXTENSION=.exe
	LINKSUFFIX=.dll
	LDFLAGS="-lmingw32 $LDFLAGS -lws2_32 -lglfw -lopengl32"
	echo "Target: mingw";
else
	LINKSUFFIX=.so
	LDFLAGS="$LDFLAGS -lc $(pkg-config --libs gl)"
  LDFLAGS="$LDFLAGS $(pkg-config --libs glfw)";
  CFLAGS="$CFLAGS $(pkg-config --cflags gl)"
  CFLAGS="$CFLAGS $(pkg-config --cflags glfw)";
	echo "Target: linux";
fi;

mkdir -p obj;
mkdir -p bin;

cd src;

SOURCES="*.c common/*.c graphics/*.c network/*.c script/*.c asset/*.c" # physics/*.c

echo "SOURCES: $SOURCES";

for f in $SOURCES; do
	obj="../obj/$(basename $f .c).o";
	echo "$f -> $obj";
	#fmod=$(stat --format=%Y $f);
	#omod=$(stat --format=%Y $obj);
        if $(test -e $obj -o $f -nt $obj); then
		echo "$CC $CFLAGS -c $f -o $obj";
		$CC $CFLAGS -c $f -o $obj;

		if $(test $? -ne 0); then # test to see if it failed
			echo "Compilation terminated";
			exit 1;
		fi;
	fi;
done;

cd ..

echo "$LD obj/*.o $LDFLAGS -o bin/il$EXTENSION"
$LD obj/*.o $LDFLAGS -o bin/il$EXTENSION

