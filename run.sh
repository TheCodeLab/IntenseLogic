if [ ! \( -e "modules" \) ]; then
    ln -s build modules;
fi;
LD_LIBRARY_PATH=build build/il -r test.lua
