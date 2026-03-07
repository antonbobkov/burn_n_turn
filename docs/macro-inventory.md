# Macro Inventory

This document catalogs every non-include-guard `#define` macro in the codebase, what logic depends on each one, and where they are used. The goal is to support replacing them with proper C++ alternatives (e.g. `constexpr`, runtime config, virtual dispatch).

---

## Central Definition File

All build-configuration macros are defined in one place:

**`code/game/dragon_macros.h`**

```cpp
#define FULL_VERSION
#define PC_VERSION
//#define KEYBOARD_CONTROLS

#ifdef KEYBOARD_CONTROLS
//#define JOYSTICK_TUTORIAL
#endif

#ifdef PC_VERSION
//#define FULL_SCREEN_VERSION
//#define LOADING_SCREEN
#endif

//#define SMALL_SCREEN_VERSION

#ifndef FULL_VERSION
#define TRIAL_VERSION
#endif

#ifdef SMALL_SCREEN_VERSION
#define FLIGHT_POWER_MODE
#endif

//#define UNDERLINE_UNIT_TEXT
```

### Current active macros (as of this writing)

| Macro | Status |
|---|---|
| `FULL_VERSION` | **ON** |
| `PC_VERSION` | **ON** |
| `TRIAL_VERSION` | OFF (implicit — only defined when `FULL_VERSION` is absent) |
| `KEYBOARD_CONTROLS` | OFF (commented out) |
| `JOYSTICK_TUTORIAL` | OFF (commented out, also requires `KEYBOARD_CONTROLS`) |
| `FULL_SCREEN_VERSION` | OFF (commented out, also only valid under `PC_VERSION`) — **never used anywhere** |
| `LOADING_SCREEN` | OFF (commented out, only valid under `PC_VERSION`) |
| `SMALL_SCREEN_VERSION` | OFF (commented out) |
| `FLIGHT_POWER_MODE` | OFF (auto-derived from `SMALL_SCREEN_VERSION`) |
| `UNDERLINE_UNIT_TEXT` | OFF (commented out) |

---

## Macro Details

### 1. `FULL_VERSION` / `TRIAL_VERSION`

These two are mutually exclusive. `TRIAL_VERSION` is automatically `#define`d when `FULL_VERSION` is absent.

**What they control:**

| File | Lines | Logic |
|---|---|---|
| `game/dragon_game_runner.cc` | ~62–70 | Loads `"levels.txt"` (full) vs `"levels_trial.txt"` (trial) |
| `game/dragon_game_runner.cc` | ~108–110 | Loads trial splash image (trial only) |
| `game/controller/level_controller.cc` | ~31–64 | Shows/hides HUD elements: score display, timer, level info, cheat indicator |
| `game/controller/level_controller.cc` | ~592–619 | Enables ghost-time mechanic and timer flash warnings (full only) |
| `game/controller/dragon_game_controller.cc` | ~307–320 | Loads different music tracks for full version |
| `game/controller/dragon_game_controller.cc` | ~108 | Loads trial splash screen (trial only) |
| `game/controller/dragon_game_controller.cc` | ~424–427 | Renders "trial" image overlay (trial only) |
| `game/controller/dragon_game_controller.cc` | ~521–550 | Creates "Buy Now" controller screen (trial only) |
| `game/controller/dragon_game_controller.cc` | ~555 | Additional full-version functionality |
| `game/critter_generators.cc` | ~39–42 | Forces first-level tutorials (trial only) |
| `game/critter_generators.cc` | ~93–96 | Princess generator always shows "first" tutorial (trial only) |
| `game/critter_generators.cc` | ~188–193 | Trader generator first-bonus flag (trial only) |

**Replacement approach:** Replace with a runtime `bool isFullVersion` flag (or equivalent enum) passed through config/settings. The `BuyNowController` and trial splash can be registered conditionally at startup rather than compiled conditionally.

---

### 2. `PC_VERSION`

Marks a desktop PC build (vs. mobile/touch). Currently always defined.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/entities.cc` | ~177–194 | Tutorial text: PC shows "click / space" instructions; others show "tap" |
| `game/dragon_game_runner.cc` | ~104–107 | Conditional sound effects on loading screen |
| `game/dragon_game_runner.cc` | ~117–142 | Mouse move/down/up event handlers — only registered on PC |
| `game/controller/level_controller.cc` | ~191–203 | Tutorial text setup for PC platform |
| `game/controller/level_controller.cc` | ~254–271 | Keyboard input handling and tutorial updates |
| `game/controller/level_controller.cc` | ~543–548 | Mouse cursor drawn only on PC (and only when not in `KEYBOARD_CONTROLS` mode) |
| `game/controller/dragon_game_controller.cc` | ~26–59 | Loading screen with platform-specific layout |
| `game/controller/dragon_game_controller.cc` | ~53–57 | Platform-specific beep sounds |
| `game/controller/dragon_game_controller.cc` | ~448 | Additional PC-only game mechanics |
| `game/controller/dragon_game_controller.cc` | ~544 | Combined PC + keyboard-controls check |

**Replacement approach:** Replace with a `Platform` enum or `bool isMobile` runtime flag. Mouse/touch event registration can be decided at startup. Tutorial text can be driven by a `ControlScheme` value.

---

### 3. `KEYBOARD_CONTROLS`

Switches the control scheme from mouse/touch to keyboard (arrow keys + space). Currently disabled.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/dragon_constants.h` | ~61–67 | Fireball counts: without keyboard = 4/2; with keyboard = 6/3 |
| `game/dragon_game_runner.cc` | ~118, 128, 138 | Disables mouse move/down/up handlers |
| `game/tutorial.cc` | ~8–23 | Tutorial strings: keyboard messages vs. click/tap messages |
| `game/tutorial.cc` | ~85 | Controls display of "flying while shooting" hint |
| `game/controller/level_controller.cc` | ~130–218 | Keyboard input: LEFT/RIGHT arrow key handling for dragon movement |
| `game/controller/level_controller.cc` | ~208–270 | Multiple keyboard control sections (shoot, move, etc.) |
| `game/controller/level_controller.cc` | ~544 | Suppresses mouse cursor drawing |
| `game/controller/level_controller.cc` | ~621 | Additional keyboard handling |
| `game/controller/dragon_game_controller.cc` | ~410–412 | Disables the traders hint in keyboard mode |
| `game/controller/dragon_game_controller.cc` | ~544 | Combined with `PC_VERSION` |

**Replacement approach:** Replace with a `ControlScheme` enum (`Mouse`, `Keyboard`, `Joystick`). Input handlers and tutorial strings should be driven by this value at runtime.

---

### 4. `JOYSTICK_TUTORIAL`

Selects joystick-specific tutorial text. Only meaningful when `KEYBOARD_CONTROLS` is also defined. Currently disabled.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/tutorial.cc` | ~9–23 | Switches tutorial message strings to joystick variants |
| `game/tutorial.cc` | ~85–87 | Controls display of the shooting-hint text |

**Replacement approach:** Fold into the `ControlScheme` enum (add `Joystick` variant). Tutorial string selection becomes a runtime lookup.

---

### 5. `SMALL_SCREEN_VERSION`

Adapts the game for small/mobile screens. Currently disabled. Enabling it also enables `FLIGHT_POWER_MODE`.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/entities.cc` | ~184–194 | Adjusts instruction text for small screens |
| `game/game_runner_interface_impl.cc` | ~13–16 | Screen resolution: `854×480` (small) vs `960×600` (normal) |
| `game/controller/dragon_game_controller.cc` | ~34–38 | Layout adjustments in loading screen |
| `game/controller/dragon_game_controller.cc` | ~63–66 | Loads `"levels_small.txt"` instead of `"levels.txt"` |

**Replacement approach:** Replace with a `ScreenSize` or resolution config value. Layout and level-file selection can be driven from screen dimensions at runtime.

---

### 6. `FLIGHT_POWER_MODE`

Special flight mechanic for small-screen devices. Auto-derived from `SMALL_SCREEN_VERSION`; never set independently.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/dragon.cc` | ~346–352 | Skips fireball-bonus collection while not in flight (special flight mechanic) |

**Replacement approach:** Replace with a runtime `bool flightPowerMode` flag, driven by `ScreenSize` config.

---

### 7. `LOADING_SCREEN`

Shows a loading screen with splash image and progress bar. Only valid under `PC_VERSION`. Currently disabled.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/controller/dragon_game_controller.cc` | ~26–59 | Draws loading screen UI: splash image + progress bar |

**Replacement approach:** Replace with a runtime flag or a dedicated `LoadingScreenController` that is optionally inserted into the controller chain at startup.

---

### 8. `FULL_SCREEN_VERSION`

Defined (commented out) under `PC_VERSION` in `dragon_macros.h`. **Never referenced anywhere else in the codebase.** Dead macro — can be deleted.

---

### 9. `UNDERLINE_UNIT_TEXT`

Debug/visual feature: draws text labels underneath in-game units. Currently disabled.

**What it controls:**

| File | Lines | Logic |
|---|---|---|
| `game/critters.cc` | ~53–56 | Draws unit label text for critters |
| `game/critters.cc` | ~161, ~172 | Additional critter label draw calls |
| `game/fireball.cc` | ~372–375 | Draws text under fireball bonus animations |

**Replacement approach:** Replace with a runtime `bool debugUnitText` flag (or a debug-overlay config), toggled via a debug menu or command-line argument.

---

## Include Guards (not subject to removal)

All header files use `#ifndef / #define / #endif` include guards. These are standard C++ practice and are not part of the macro-removal effort. They can optionally be replaced with `#pragma once`, but that is a separate concern.

---

## Dependency Graph

```
FULL_VERSION ────────────────────── content/licensing features
    └── (absent) → TRIAL_VERSION ── trial splash, Buy Now screen, tutorial flags

PC_VERSION ──────────────────────── mouse handlers, PC layout, beep sounds
    ├── FULL_SCREEN_VERSION         (DEAD — never used)
    └── LOADING_SCREEN ──────────── loading screen UI

KEYBOARD_CONTROLS ───────────────── keyboard input, fireball counts, tutorial text
    └── JOYSTICK_TUTORIAL ────────── joystick tutorial variants

SMALL_SCREEN_VERSION ────────────── resolution (854×480), levels_small.txt, layout
    └── FLIGHT_POWER_MODE ────────── flight bonus mechanic
```
