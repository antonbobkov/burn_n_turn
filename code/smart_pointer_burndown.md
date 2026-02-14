instructions for target_class

- for each object creation make_smart(new target_class(...)) we would like to remove all instances of smart_pointer<target_class> and replace it by:
  - one instance std::unique_ptr<target_class> where the object is owned for its lifetime
  - target_class* everywhere else

Determine lifetime of the object. Find where to put unique pointer. If it is stored in an array of base classes, okay to keep as smart_pointer in there, but all other spots should still be target_class*

At the end, if no uses of smart pointers are left for the class, and the class derives directly from SP_Info, remove SP_Info as the parent.

If you cannot detemine lifetime of the object, don't change any code and let me know