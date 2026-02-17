# Stack Research

**Domain:** C++ ownership migration (custom ref-counted pointer → std::unique_ptr)
**Researched:** 2025-02-16
**Confidence:** HIGH

## Recommended Stack

### Core Technologies

| Technology   | Version | Purpose              | Why Recommended                    |
|-------------|---------|----------------------|------------------------------------|
| C++14       | (fixed) | Language standard    | Project constraint; sufficient for  |
|             |         |                      | std::unique_ptr and move semantics |
| std::unique_ptr | C++11+ | Single ownership     | Standard, zero-cost; replaces      |
|             |         |                      | custom smart_pointer where one    |
|             |         |                      | owner exists.                      |
| Raw pointer | —       | Non-owning reference | For observers; no ownership transfer|
| std::make_unique | C++14 | Construction         | Prefer over unique_ptr(new T());   |
|             |         |                      | exception-safe, clearer.           |

### Supporting / Tooling

| Tool / practice | Purpose              | When to Use                    |
|-----------------|----------------------|--------------------------------|
| Build + tests   | Regression safety    | After every migration step     |
| simulation_test | Game/smart-pointer   | Primary correctness gate       |
|                 | balance assertions    | (per project rules).           |
| clang-tidy      | modernize-make-unique| Optional; after introducing    |
| (optional)      |                      | unique_ptr to normalize style. |

### Development Workflow

| Practice        | Purpose                    | Notes                          |
|-----------------|----------------------------|--------------------------------|
| One class at a  | Avoid mixed ownership and  | Per smart_pointer_burndown.md; |
| time (burndown)  | unclear lifetimes.         | determine lifetime, then change|
| Bottom-up order | Clear ownership chain;     | Leaf types first, then owners. |
|                 | less rework.               |                                |

## What NOT to Use

| Avoid                    | Why                         | Use Instead              |
|--------------------------|-----------------------------|---------------------------|
| New custom ref-counting  | Reinventing standard;       | std::unique_ptr + raw ptr |
| for single-owner cases   | harder to reason about.     | or shared_ptr if truly shared |
| shared_ptr "to be safe"  | Hides single-ownership;     | unique_ptr + raw ptr      |
| everywhere               | cost and API noise.         | where only one owner exists |
| Raw pointer for ownership| No RAII; leaks and         | unique_ptr at the single  |
|                          | use-after-free risk.        | owning site.              |

## Alternatives Considered

| Recommended        | Alternative     | When to Use Alternative        |
|--------------------|-----------------|--------------------------------|
| unique_ptr + T*    | Keep smart_pointer in base | Only for containers of base   |
|                    | class arrays    | pointers (per burndown doc).   |
| make_unique        | unique_ptr(new T())        | When make_unique not possible |
|                    |                 | (e.g. private ctor, custom alloc). |

## Sources

- C++ Core Guidelines (R.11, F.7, F.26, R.30) — ownership and parameters.
- Refactoring into unique_ptr (C++ Stories, isocpp.org) — incremental steps.
- Project: code/smart_pointer_burndown.md, .planning/codebase/ARCHITECTURE.md.

---
*Stack research for: C++ smart pointer migration*
*Researched: 2025-02-16*
