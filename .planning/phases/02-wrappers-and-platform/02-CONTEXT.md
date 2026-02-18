# Phase 2: Wrappers and platform types - Context

**Gathered:** 2025-02-16
**Status:** Ready for planning

<domain>
## Phase Boundary

Migrate GraphicalInterface, SoundInterface, FontWriter, and their
implementations (SDL, mocks) and proxies to single ownership: unique_ptr at
owner, T* elsewhere. No new features; ownership only for these wrapper/platform
types.
</domain>

<decisions>
## Implementation Decisions

### Owner location
- **FontWriter:** Single owner in **TowerDataWrap**. One unique_ptr there;
  all other code uses FontWriter*.
- **GraphicalInterface and SoundInterface:** Single owners in the **_main.cc**
  file (each executable's main owns its own GUI and sound). Not in
  ProgramEngine.

### Claude's Discretion
- Migration order: by layer (interfaces then implementations) or by vertical
  slice (one subsystem end-to-end then next); which subsystem first (GUI,
  sound, font).
- Test mocks: migrate in lockstep with real implementations or treat
  separately.
- Recording skipped types: what to record (type name, rationale, both) and
  where (e.g. .planning file, in-code, both) for DOC-01.
</decisions>

<specifics>
## Specific Ideas

- Ownership split: GUI/sound at entry (main), font inside TowerDataWrap.
</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.
</deferred>

---
*Phase: 02-wrappers-and-platform*
*Context gathered: 2025-02-16*
