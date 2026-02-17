# Pitfalls Research

**Domain:** C++ ownership migration (custom ref-counted → std::unique_ptr)
**Researched:** 2025-02-16
**Confidence:** HIGH

## Critical Pitfalls

### Pitfall 1: Dangling raw pointer after unique_ptr is destroyed

**What goes wrong:**
Code holds a T* that referred to an object owned by a unique_ptr; the
unique_ptr is destroyed (e.g. scope exit, container resize) and the raw
pointer is used later → use-after-free.

**Why it happens:**
Non-owning sites keep a raw pointer while the single owner is temporary or
stored in a container that gets modified.

**How to avoid:**
- Ensure the single owner’s lifetime is at least as long as any raw pointer
  use (e.g. owner lives in ProgramEngine or runner for the whole run).
- Don’t store long-lived T* that point into short-lived unique_ptrs.

**Warning signs:**
- Raw pointer stored in a member or global while the unique_ptr is in a
  local or short-lived container.
- Passing T* into a callback that is invoked after the owner might be gone.

**Phase to address:**
Every migration phase: when placing unique_ptr, check all T* uses and
confirm owner lifetime.

---

### Pitfall 2: Two unique_ptrs for the same object (double delete)

**What goes wrong:**
Two different places each hold unique_ptr<T> to the same T (e.g. one from
move, one from a leftover copy or wrong assignment) → double delete.

**Why it happens:**
Copying unique_ptr is deleted; but during refactors, one site might be
updated to unique_ptr while another still “gives” the same pointer (e.g. via
release() and then another wrap), or ownership is mistakenly duplicated.

**How to avoid:**
- Only one owning site: one unique_ptr per object.
- When transferring ownership, use move (std::move(ptr)) or return
  unique_ptr from factory; never duplicate the same pointer into two
  unique_ptrs.

**Warning signs:**
- Manual release()/reset() with the same raw pointer in two places.
- “Sharing” a pointer by wrapping the same raw ptr in unique_ptr twice.

**Phase to address:**
Every phase when introducing unique_ptr; review all creation and assignment
paths.

---

### Pitfall 3: Migrating a type before its dependencies

**What goes wrong:**
You change type A to unique_ptr + T*, but A is owned or referenced by type B
that still uses smart_pointer<A>. You end up mixing smart_pointer and
unique_ptr for A, or redoing A when B is migrated.

**Why it happens:**
Top-down or random order; ownership and call sites span multiple types.

**How to avoid:**
- Follow bottom-up order: migrate types that don’t own other
  smart_pointer types first, then their owners.
- Per smart_pointer_burndown.md: if lifetime isn’t clear, don’t change
  that class yet; report and resolve.

**Warning signs:**
- Many compile errors in unrelated files after changing one type.
- Need to temporarily keep both smart_pointer<T> and unique_ptr<T> for the
  same T.

**Phase to address:**
Roadmap phase ordering; each phase should migrate types whose dependents are
already migrated or don’t use smart_pointer.

---

### Pitfall 4: Removing SP_Info while smart_pointer<T> is still used

**What goes wrong:**
You remove SP_Info from class T while some code still has smart_pointer<T>
→ compile or link errors, or undefined behavior if SP_Info was part of the
object layout.

**Why it happens:**
Incomplete grep or missed call site; or a branch (e.g. test) still using
smart_pointer<T>.

**How to avoid:**
- Remove SP_Info only when there are zero uses of smart_pointer<T> (and
  make_smart(new T(...))).
- Search entire codebase (and tests) for smart_pointer<T>, make_smart, and
  SP_Info before removing base class.

**Warning signs:**
- Build or tests fail right after removing SP_Info from T.

**Phase to address:**
Final step per class in each phase: verify no smart_pointer<T> left, then
remove SP_Info.

---

### Pitfall 5: Skipping tests between large changes

**What goes wrong:**
You migrate several classes, then run tests and get failures; hard to know
which change introduced the bug.

**Why it happens:**
Time pressure or “one more file” habit.

**How to avoid:**
- Run build and tests after every migrated class (or small, coherent group).
- Keep simulation_test and ctest as the gate; fix before proceeding.

**Warning signs:**
- Multiple files modified without a test run in between.

**Phase to address:**
Every phase: define “done” to include “build + all tests pass.”

## Technical Debt Patterns

| Shortcut           | Benefit        | Long-term Cost      | When Acceptable   |
|--------------------|----------------|---------------------|-------------------|
| Keep smart_pointer | Less change    | Mixed semantics;    | Only for base-class|
| in base-class array| in one place   | never fully remove  | container (per doc)|
| Raw pointer for     | “Faster”       | Use-after-free risk | Never; use        |
| ownership           |                |                     | unique_ptr at owner|
| Defer tests         | Speed          | Regressions slip    | Never             |

## Pitfall-to-Phase Mapping

| Pitfall                    | Prevention phase      | Verification                    |
|----------------------------|------------------------|---------------------------------|
| Dangling T*                | Every migration step   | Review owner lifetime vs T* use |
| Double unique_ptr          | Every migration step   | Single creation/ownership path  |
| Wrong dependency order     | Roadmap phase order    | Bottom-up; no rework on same T  |
| SP_Info removed too early  | Per-class final step   | grep smart_pointer<T> = 0      |
| No tests between steps     | Every phase            | ctest + simulation_test pass   |

## Sources

- code/smart_pointer_burndown.md
- PROJECT.md (Core Value: tests must pass)
- C++ Core Guidelines; common unique_ptr migration mistakes

---
*Pitfalls research for: C++ smart pointer migration*
*Researched: 2025-02-16*
