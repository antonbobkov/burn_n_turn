# Feature Research

**Domain:** C++ ownership migration (custom ref-counted → std::unique_ptr)
**Researched:** 2025-02-16
**Confidence:** HIGH

## Feature Landscape

### Table Stakes (Migration Must Have These)

Aspects the migration must cover or the codebase stays inconsistent or broken.

| Feature              | Why Expected              | Complexity | Notes                          |
|----------------------|---------------------------|------------|--------------------------------|
| Lifetime analysis    | One unique_ptr owner      | MEDIUM     | Per-class; find creation and    |
| per class            | requires knowing lifetime.|            | single logical owner.          |
| unique_ptr at owner  | Single owning location    | LOW        | One place holds unique_ptr;    |
| only                  | per object.               |            | rest use T*.                   |
| Raw pointers elsewhere| Non-owners must not      | LOW        | Parameters, members, returns   |
|                      | own; T* or T&.             |            | that don't transfer ownership. |
| Build + tests pass   | No behavior change.        | LOW        | simulation_test and ctest     |
| after each step      |                            |            | after every change.            |
| Remove SP_Info when  | Clean end state per class.| LOW        | When no smart_pointer<T> left.  |
| class is done        |                            |            |                                |

### Differentiators (Quality of Migration)

| Feature           | Value Proposition        | Complexity | Notes                    |
|-------------------|-------------------------|------------|--------------------------|
| Bottom-up order   | Clear ownership chain;  | LOW        | Leaf types first.        |
|                   | less rework.             |            |                          |
| make_unique       | Exception safety,       | LOW        | Prefer over              |
| where possible    | clearer intent.          |            | unique_ptr(new T()).     |
| Optional clang-tidy| Consistent style       | LOW        | After unique_ptr in use. |

### Anti-Features (Often Tempting, Problematic)

| Feature        | Why Requested     | Why Problematic        | Alternative              |
|----------------|-------------------|------------------------|--------------------------|
| Big-bang switch| "Do it all once"  | Hard to debug; high    | Incremental, one class   |
|                |                   | risk of subtle bugs.   | (or small group) at a time |
| shared_ptr     | "Safe everywhere"  | Hides single ownership;| Use unique_ptr + raw ptr |
| everywhere     |                   | cost and API noise.    | where only one owner.    |
| Skipping tests  | Speed             | Regressions slip;      | Run tests every step.    |
| between steps  |                   | hard to bisect.         |                          |

## Feature Dependencies

```
Lifetime analysis
    └──requires──> unique_ptr placement
                       └──requires──> Replace smart_pointer with T*
                                          └──requires──> Remove SP_Info (when done)

Build + tests pass ──required at every step──> All of above
Bottom-up order ──enhances──> Lifetime analysis (clearer ownership)
```

### Dependency Notes

- **Lifetime analysis requires unique_ptr placement:** Must decide the single
  owner before changing types.
- **Replace smart_pointer with T*:** All non-owning uses become raw pointers.
- **Remove SP_Info:** Only when no smart_pointer<T> remains for that class.
- **Bottom-up order enhances lifetime analysis:** Leaf types have fewer
  dependents; ownership is easier to see.

## MVP Definition (Migration v1)

### Launch With (v1)

- [ ] Every class using smart_pointer migrated per burndown (one unique_ptr
  owner, T* elsewhere; base-class arrays exception as in doc).
- [ ] SP_Info removed from every migrated class.
- [ ] smart_pointer implementation removed (headers, impl, tests, SP_Info).
- [ ] Build and all tests pass (simulation_test in particular).

### Out of Scope (v1)

- [ ] Introducing std::shared_ptr for any current smart_pointer use.
- [ ] Changing C++ standard or build system beyond what’s needed for migration.

## Feature Prioritization

| Feature           | Value   | Cost   | Priority |
|-------------------|---------|--------|----------|
| Per-class burndown| HIGH    | MEDIUM | P1       |
| Bottom-up order   | HIGH    | LOW    | P1       |
| Tests after each  | HIGH    | LOW    | P1       |
| Remove SP_Info    | HIGH    | LOW    | P1       |
| Remove smart_pointer| HIGH  | MEDIUM | P1       |
| make_unique       | MEDIUM  | LOW    | P2       |
| clang-tidy pass   | LOW     | LOW    | P3       |

## Sources

- code/smart_pointer_burndown.md
- PROJECT.md (Core Value, Active requirements)
- C++ Core Guidelines; refactoring articles (unique_ptr migration)

---
*Feature research for: C++ smart pointer migration*
*Researched: 2025-02-16*
