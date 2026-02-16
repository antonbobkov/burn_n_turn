# Code conventions (code/)

From `code/how_to_code.md` and codebase patterns. Scope: `code/` only.

## Style

- **Line length**: Keep lines under 80 characters (workspace rule in
  `.cursor/rules/Code-style.mdc`).
- **Comments**: Simple, non-technical language where possible (same rule).
- **Class layout**: Class name CamelCase; function names CamelCase; variables
  snake_case. Public first, private last. Member variables end with
  underscore; other function parameters do not end with underscore.
- **Encapsulation**: No public member variables; use getters/setters.

## Implementation location

- **Non-template classes**: Functions longer than one line are implemented in
  the `.cc` file; headers keep declarations. In `.h` files, prefer
  forward declarations for pointer/smart-pointer types; include the header
  that declares the type only when needed (e.g. base class, by-value types).
  See `code/identify-dependencies.md` for narrowing includes.
- **New headers**: After adding a new `.h`, add explicit includes in all
  `.h`/`.cc` that use its declarations (workspace rule).

## Naming and parameters

- **Functions**: Prefer pointer parameters to non-const references where
  appropriate.
- **Helpers in .cc**: Use anonymous namespaces for helper functions in
  implementation files.

## Exceptions and errors

- **Custom hierarchy**: `MyException` in `code/utils/exception.h` (name,
  class, function stack, optional inherited message). Subclasses override
  `GetErrorMessage()`. Examples: `MatrixException`, `SimpleException`,
  `ImageException`, `SdlImageException`.
- **Throwing**: Provide useful debugging information (class name, function,
  context such as `MatrixErrorInfo`).

## Memory and types

- **Avoid**: Explicit `new`/`delete`; prefer `std::unique_ptr` when possible.
- **smart_pointer**: Custom ref-counted pointer used widely; objects inherit
  `SP_Info`. Migration toward `unique_ptr` for single-owner cases is
  documented in `code/smart_pointer_burndown.md`.
- **Avoid**: Downcasts from base to derived; avoid `unsigned` when possible
  (per how_to_code).

## Testing

- **Unit tests**: Avoid writing/reading real files; use `InMemoryFileManager`
  or similar so tests don’t touch the filesystem.

## Build and includes

- **Includes**: Non-std includes should be narrow and specific; only include
  what is needed. Pointers/smart pointers and types in containers can often
  use forward declarations; include the defining header for base classes and
  non-pointer members.
