To build on windows, from the repository root:

`cmake --preset debug`

`cmake --build --preset debug -j4`

To run the tests:

`ctest --preset integration -j4`

recursive clang

`for /r %f in (*.h *.cpp *.cc) do clang-format -i %f`
