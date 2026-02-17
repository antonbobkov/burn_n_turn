# Architecture Research

**Domain:** C++ ownership migration (ref-counted → unique_ptr)
**Researched:** 2025-02-16
**Confidence:** HIGH

## Standard Architecture (Migration Structure)

### Component Order (Bottom-Up)

```
┌─────────────────────────────────────────────────────────────┐
│  Leaf / value-like types (no smart_pointer to other game types)│
│  e.g. simple entities, drawers, animations used in one place  │
├─────────────────────────────────────────────────────────────┤
│  Mid-level: types that own leaf types                         │
│  e.g. controllers, generators, entities that own animations   │
├─────────────────────────────────────────────────────────────┤
│  Runners / engines: own controllers, ProgramEngine, etc.      │
├─────────────────────────────────────────────────────────────┤
│  Entry points: main, simulation_test, GetGameRunner           │
│  (last to change; they hold unique_ptr to top-level objects)  │
└─────────────────────────────────────────────────────────────┘
```

### Responsibility per Layer

| Layer        | Responsibility              | Typical change                          |
|-------------|-----------------------------|-----------------------------------------|
| Leaf types  | No ownership of other       | unique_ptr at creator; T* in call sites;|
|             | smart_pointer types         | remove SP_Info when done.               |
| Mid-level   | Own leaf types; passed      | Hold unique_ptr for owned; receive T*   |
|             | down or stored in containers| or refs; remove SP_Info when done.      |
| Runners     | Own controllers, UI, etc.  | unique_ptr for runner-owned objects;    |
|             |                             | ProgramEngine etc. own their members.  |
| Entry points| Construct top-level objects | make_unique at creation; pass raw ptr   |
|             |                             | or ref into engine/runner.             |

## Recommended Project Structure (Order of Work)

Not folder layout—order of migration across existing code/ layout:

1. **utils / game_utils** — Types that are used as dependencies (Event,
   MessageWriter, draw/sound helpers) but are leaf-like in the sense they
   don’t own other smart_pointer game objects. Clarify ownership at
   ProgramEngine or runner level.
2. **wrappers** — GUI/sound interfaces and implementations (if they use
   smart_pointer). Often owned by ProgramEngine or main.
3. **game (entities, fireball, critters)** — Leaf entities first (e.g.
   AnimationOnce, BonusScore), then entities that own them (critters,
   generators), then level/controller-owned structures.
4. **game (controllers, runner)** — Controllers own view entities; runner
   owns current controller. Migrate after the types they own.
5. **simulation / main** — Last: they create ProgramEngine, runner, etc.
   and hold unique_ptr at top level.

## Data Flow (Ownership Flow)

```
Entry (main / simulation_test)
    │
    ├── unique_ptr<ProgramEngine> (or similar)
    │       │
    │       ├── unique_ptr<Event>, unique_ptr<MessageWriter>
    │       ├── unique_ptr<GraphicalInterface>, unique_ptr<SoundInterface>
    │       └── ...
    │
    └── unique_ptr<DragonGameRunner> (or GameRunner)
            │
            └── unique_ptr<DragonGameController> (current controller)
                    │
                    └── unique_ptr<LevelController> / MenuController / ...
                            │
                            └── T* or unique_ptr for entities, UI elements
```

- **Ownership:** One unique_ptr per object at the single owning site.
- **Borrowing:** Everyone else uses T* or T& (no ownership).

## Anti-Patterns

### 1. Top-down before leaves

**What people do:** Change ProgramEngine and runner first.
**Why it’s wrong:** Ownership of nested smart_pointer types is still unclear;
you end up changing the same types repeatedly.
**Do this instead:** Migrate leaf types first, then their owners.

### 2. Keeping smart_pointer and unique_ptr for the same type

**What people do:** Some call sites use smart_pointer<T>, others unique_ptr<T>.
**Why it’s wrong:** Mixed ownership semantics; easy to double-delete or leak.
**Do this instead:** One class at a time; when done, only unique_ptr + T* for
that type.

### 3. Raw pointer for ownership

**What people do:** Store T* in the “owner” and never delete or use unique_ptr.
**Why it’s wrong:** Leaks or unclear who deletes.
**Do this instead:** The single owner holds unique_ptr<T>; others hold T*.

## Integration Points

| Boundary              | Communication        | Notes                          |
|-----------------------|----------------------|---------------------------------|
| Factory / constructor | Return unique_ptr    | Transfer ownership to caller.   |
| Callbacks / listeners | Pass T* or T&        | No ownership transfer.          |
| Containers of base    | Per burndown doc:   | May keep smart_pointer in that  |
| class pointers       | exception allowed    | container only; rest use T*.    |

## Sources

- code/smart_pointer_burndown.md
- .planning/codebase/ARCHITECTURE.md (existing layers)
- PROJECT.md (bottom-up decision)

---
*Architecture research for: C++ smart pointer migration*
*Researched: 2025-02-16*
