# General / file_utils / exception split — execution summary

Refactor: move exception logic to utils/exception.{h,cpp}, file logic to
utils/file_utils.{h,cpp}. Plan: move_file_logic_to_file_utils_d1f03367.plan.md.

---

### Step 1: Add utils/exception.h and utils/exception.cpp

- **What was done:** Created utils/exception.h (crefString, MyException,
  SimpleException). Created utils/exception.cpp (MyException::GetHeader,
  ResetName, ctor, AddFnName, InheritException, GetDescription). No removals
  from General yet.
- **Issues:** None.
- **How you solved them:** N/A.
- **What you learned:** exception.h needs no SmartPointer or other Gui types;
  only &lt;string&gt;.
- **Future avoidance:** When introducing a new header that will be included by
  many TUs, keep its includes minimal to avoid pulling in large dependency
  trees.

### Step 2: Add utils/file_utils.h

- **What was done:** Created utils/file_utils.h with ParsePosition,
  ParseGrabNext, ParseGrabLine; OutStreamHandler, InStreamHandler; FileManager,
  FunnyFileManager, StdFileManager; FilePath and operators; RecordKeeper,
  Record, RecordCollection; Separate; and template definitions for
  Record::Read/Write. Includes exception.h and SmartPointer.h. No removals yet.
- **Issues:** None.
- **How you solved them:** N/A.
- **What you learned:** file_utils.h must include exception.h so that
  RecordKeeper::ReadDef/WriteDef (in .cpp) can throw SimpleException.
- **Future avoidance:** When moving template class methods that stay in the
  header, copy the full template body; call sites will instantiate from the
  new header.

### Step 3: Add utils/file_utils.cpp

- **What was done:** Created utils/file_utils.cpp with all definitions moved
  from General.cpp: ParsePosition, ParseGrabNext, ParseGrabLine; FilePath
  ctor and methods; StdFileManager/FunnyFileManager WriteFile/ReadFile;
  operator<< and operator>> for FilePath; Separate; RecordKeeper::ReadDef/
  WriteDef (throwing SimpleException); RecordCollection methods. No removals
  yet.
- **Issues:** None.
- **How you solved them:** N/A.
- **What you learned:** file_utils.cpp only needs file_utils.h (and fstream);
  SimpleException comes via file_utils.h -> exception.h.
- **Future avoidance:** When moving .cpp definitions, include the minimal
  header set so the new module does not depend on the old one.

### Step 4: Trim General.h

- **What was done:** Removed crefString; MyException and SimpleException;
  ParsePosition, ParseGrabNext, ParseGrabLine; OutStreamHandler through
  Separate; Record template bodies. Added
  #include "exception.h" after SmartPointer.h and #include "file_utils.h"
  before closing namespace. Updated header comment.
- **Issues:** None.
- **How you solved them:** N/A.
- **What you learned:** General.h now only defines Gmin/Gmax/Gabs, S(T),
  Timer, Index, IndexKeeper, GuiKeyType; exception and file_utils are
  re-exported via includes.
- **Future avoidance:** When removing a large block, do it in one or two
  search-replace chunks so the line numbers in the plan still match
  approximately for the next file (General.cpp).

### Step 5: Trim General.cpp

- **What was done:** Removed all MyException member definitions;
  ParsePosition, ParseGrabNext, ParseGrabLine; FilePath ctor and all methods;
  StdFileManager/FunnyFileManager WriteFile/ReadFile; operator<< and
  operator>> for FilePath; Separate; RecordKeeper::ReadDef/WriteDef;
  RecordCollection::NewRecordKeeper, Read, Write. Removed #include
  &lt;fstream&gt;. Kept only Index copy ctor, dtor, operator=.
- **Issues:** None.
- **How you solved them:** N/A.
- **What you learned:** General.cpp now only compiles Index; General.h
  pulls in exception.h and file_utils.h so the TU still has full types.
- **Future avoidance:** Before deleting a block, confirm no remaining
  references in the same file to the deleted symbols (e.g. Index uses
  no file or exception types).

### Step 6: Build configuration

- **What was done:** Added exception.h, exception.cpp, file_utils.h,
  file_utils.cpp to add_library(Utils ...) in utils/CMakeLists.txt.
- **Issues:** None.
- **How you solved them:** N/A.
- **What you learned:** New sources must be listed in CMakeLists so they
  are compiled and linked into the Utils library.
- **Future avoidance:** After adding new .cpp files, run cmake and build
  before declaring the step done.

### Step 7: Build and verify

- **What was done:** Ran cmake and mingw32-make from build dir. Fixed
  include order in General.h; re-added file_utils.h include.
- **Issues:** (1) Include order: file_utils.h was included at the end of
  General.h *inside* namespace Gui, so when General.cpp was compiled,
  file_utils.h was parsed inside Gui and std::string / std::istream
  were looked up as Gui::std::*, causing many errors. (2) After moving
  the include to the top of General.h, file_utils.h was accidentally
  omitted from the includes (only exception.h was present), so
  GuiGenHdr.h failed with "expected ')' before '&'" at FontWriter(FilePath
  &fp) because FilePath was not visible.
- **How you solved them:** (1) Moved #include "file_utils.h" to the top
  of General.h (with exception.h), before "namespace Gui {", so
  file_utils.h is processed in global scope and its own "namespace Gui { }
  " is correct. (2) Re-added #include "file_utils.h" after exception.h
  in General.h so FilePath and other file types are visible to all
  includers of General.h.
- **What you learned:** Including a header that uses std:: inside
  another namespace (e.g. inside "namespace Gui {") causes std:: names
  to be looked up in that namespace (Gui::std), breaking the build.
  Headers that define types in a namespace must be included outside
  that namespace so their namespace block is at file scope.
- **Future avoidance:** When re-exporting a header (General.h including
  file_utils.h), include it at the top of the file, before opening
  the re-exporting namespace, so the included header's own namespace
  is correct. Before committing, run a full build of all targets, not
  just the one you changed; the Utils library built successfully while
  the wrappers (which also include General.h) failed until file_utils.h
  was present.

---
