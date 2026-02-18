- class style guide
  - class name is CamelCase
  - function names are CamelCase
  - variables names are snake_case
  - public first, private last
  - comment above each non-trivial/non-obvious member function and member variable
  - member variables end with an underscore
  - none of the other function arguments end in underscore
  - no public variables, everything should be controlled via public member functions. Use getters and setters if needed.
  - if non-template, functions > 1 line should be implemented in cpp file

- when throwing exceptions, provide useful debugging information

- unit tests
  - avoid writing/reading files

- avoid downcasts (from parent to derived classes)
- avoid explicit new/delete. Use std::unique_ptr instead when possible
- avoid using unsigned int/long when possible
- use anonymous namespaces for helper functions in .cpp files

- functions
  - prefer pointers to non-const references

- non-standard-library includes
  - in .h files: don't have includes for pointer/smart-pointer classes; just forward declare them. Have explicit includes for other classes used. Move function definition to c++ file, if it results in fewer includes
  - in .cpp file: have a explicit include for each external class/function used