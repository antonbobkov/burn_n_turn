To build on windows:

go to /build/ directory and then run:

`cmake ../code -G "MinGW Makefiles" && mingw32-make`

recursive clang

`for /r %f in (*.h *.cpp *.cc) do clang-format -i %f`
