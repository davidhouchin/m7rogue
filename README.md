# m7rogue
This is my work on the Roguelike C++ tutorial provided [here](http://codeumbra.eu/complete-roguelike-tutorial-using-c-and-libtcod-part-1-setting-up).
I really enjoyed this tutorial and the approach it took to building the game,
but was looking to extend it, as well as get the save/load functionality fully working.

It compiles on Linux, Windows, and OSX, but the OSX version does not save/load correctly
due to an issue with the OSX port of libtcod.

To compile, you will need to copy the include directory included with the libtcod library
to the root directory of the repo, and the libtcod and libtcodxx libraries associated
with your platform.

For Windows, I recommend the SDL 1.2.15 release [here](https://www.libsdl.org/download-1.2.php) as
the one with libtcod seems to trip virus scanners.
You will need MinGW (NOT 64) to compile it.
