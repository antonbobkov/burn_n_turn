We would like to make the set of includes as narrow and specific, only include what is needed and nothing else.
Ignore std:: classes and standard library includes #include<...>

- Look over the types used in the .h file. Determine for which ones do we need some include.
- Pointers, smart pointers, types in vectors/lists don't need includes, they can be forward-declared
- Typically we need includes for non-pointer variables and for class parents

Produce a table
- column 1: types that would require an include (just type name, nothing else)
- column 2: .h file that explicitly declares this type (just file name, nothing else)