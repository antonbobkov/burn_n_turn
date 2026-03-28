# Game Mechanics Reference

Tower defense: the dragon defends castles from waves of enemies while rescuing princesses.

---

## Win & Lose Conditions

**Win a level:** Fill every castle with 4 princesses. The moment the last princess is delivered and all castles hit 4, the level ends immediately.

**Win the game:** Complete all 9 levels.

**Lose:** A castle is destroyed (see Castle mechanics below). A 3-second countdown starts, then the game over screen appears.

---

## Dragon (Player)

The player controls a dragon that flies around the map, shoots fireballs, and delivers princesses to castles.

**States:** Flying (airborne) or Perched (sitting on a castle tower)

**Fireballs:**
- Starts with 4 (mouse) or 6 (keyboard) fireballs
- Regenerates one fireball automatically over time
- Collecting bonus pickups upgrades fireball behavior (see Bonuses)

**Controls:**
- Mouse: aim and click to fire; click tower to land/take off
- Keyboard: q/w/e/d/c/x/z/a for 8-direction movement, space to fire/land

**Carrying:**
- Dragon picks up princesses and traders by flying into them
- Delivering a princess to a free castle stores her there (protection against one knight hit)
- Delivering a trader to a castle grants a fireball bonus upgrade

---

## Movement: Roads vs. Castles

Enemies and NPCs navigate differently:

**Walk directly toward a castle** (pick a random castle and pathfind to it):
- Knights
- Skeletons
- Golems

**Spawn on a road and follow it** (move along road direction, no castle targeting):
- Princesses
- Traders
- Mages

**Random drift** (no roads, no castle targeting):
- Slimes
- MegaSlimes

Roads are defined per-level in `assets/levels.txt` as vertical or horizontal corridors. Princesses, traders, and mages spawn at a random position on a random road and move along it.

---

## Enemies

### Knight (K)
- Walks directly toward a random castle
- 1 hit to kill

### Skeleton (S)
- Walks directly toward a random castle
- Also kills princesses and traders on contact
- 1 hit to kill

### Golem (W)
- Walks directly toward a random castle at half knight speed
- 70 hits to kill; pushed back one step per fireball hit
- Immune to chain explosions
- Spawned at level timer expiry on levels 7+

### Ghost (G)
- Knights spawned during Ghost Mode are ghosts: same castle-pathfinding behavior, but 1.3× faster
- 1 hit to kill

### Mage (M)
- Spawns on a road and follows it
- Sets Angry Mode when killed
- A non-angry mage just walks — it does nothing else
- An angry mage casts spells: every ~12 seconds on average (random per-frame chance), it stops and summons a burst of skeletons, then resumes walking. It will not cast while standing within 60px of a castle.
  - Levels ≤6: 4 skeletons per cast
  - Levels 7–9: 6 skeletons per cast
  - Level 10+: 8 skeletons per cast
- On death at levels 7+, also summons slimes

### Slime (L)
- Drifts randomly; not targeted at castles or roads
- Every ~0.5 seconds, 25% chance to fully randomize velocity; occasionally drifts toward the center of the map
- 1 hit to kill
- Destroyed on contact with a knight (no fireball needed)
- Destroys bonus pickups on contact
- When a fireball hits a slime, it spawns 2 child slimes at generation+1
- When the global slime count hits the cap (`nSlimeMax`), all slimes are mass-killed and a MegaSlime spawns at their centroid
- Global cap: 100 (raised to 200 by the Nuke bonus)

### MegaSlime (E)
- Larger slime; same random drift movement as regular slimes
- 30 hits to kill
- Jump/land animation cycle; velocity randomizes on jump, stops on land
- Also destroys bonus pickups on contact
- Spawned by the MegaSliminess generator, or when the slime mass-kill triggers

---

## Friendly NPCs

### Princess (P)
- Spawns on a road and follows it
- Killed on contact by skeletons
- Dragon picks her up and delivers her to a free castle
- A princess stored in a castle absorbs one knight hit before the castle is destroyed

### Trader (T)
- Spawns on a road and follows it
- Killed on contact by skeletons
- Dragon picks him up and delivers him to a castle for a fireball bonus upgrade
- First trader drops a labeled bonus ("loot")

---

## Castles

Castles store rescued princesses and are the dragon's landing pads. The win condition is filling every castle to 4 princesses.

**Delivering to a castle:**
- The dragon lands by flying into a castle
- If carrying a princess, she is added to the castle's count
- If carrying a trader, the dragon receives a fireball bonus upgrade
- A castle must be free (dragon not already perched) to receive a princess

**When a knight reaches a castle:**
- Golem ('W'): always destroys the castle regardless of princess count
- Regular knight ('K') with dragon perched: one princess is lost; she flees the castle as a live princess entity that can be recaptured
- Regular knight ('K') with no dragon perched: all stored princesses flee at once as live entities
- Regular knight ('K') hitting an empty castle: castle is destroyed

**Castle destruction:**
- Plays destruction animation and stops the music
- Ejects the dragon if perched
- Starts the 3-second lose countdown

**Visuals:** The castle sprite reflects the current princess count (0–4 frames). Counts above 4 display as the 4-princess frame.

---

## Fireballs & Bonus Upgrades

### Base Fireball
- Destroys most enemies in 1 hit
- Disappears on hitting an enemy or leaving the screen

### Bonus Pickups
Bonuses appear on the ground as animated icons. They blink when about to expire and disappear after ~60 seconds if uncollected. Slimes and MegaSlimes destroy them on contact.

| Bonus | Effect |
|-------|--------|
| **Pershot** | +1 fireball per shot |
| **Laser** | Fires a laser beam that passes through enemies |
| **Big** | Larger fireball radius |
| **Total Num** | +2–3 max fireballs |
| **Explode** | Fireball triggers a chain explosion on hit |
| **Split** | Fireball splits into extra balls on hit |
| **Set on Fire** | Ignites knights; burning knights spread fire to neighbors |
| **Ring** | Orbiting fireballs circle the dragon (only collectable from a tower) |
| **Nuke** | One-shot kills all enemies; raises slime cap to 200 |
| **Speed** | Increases fireball speed |
| **Regen** | Faster fireball regeneration |

Bonus duration scales down at higher levels (~15% shorter at levels 4–6, ~30% shorter at levels 7+).

### Special Fireball Behaviors

**Burning Knight (Set on Fire):**
- A knight hit by a "set on fire" fireball runs in panic and spreads fire to nearby knights
- Chain decrements on each spread; burns out after 15 frames

**Chain Explosion (Explode):**
- An expanding ring spawned on fireball impact
- Damages any enemy inside the expanding radius each frame
- Can chain-explode again (limited generations)
- Does not affect Golems or MegaSlimes

**Circular Fireball (Ring):**
- Orbits the dragon at a fixed radius
- Damages enemies it contacts while circling
- Lasts 2× the normal bonus duration

---

## Enemy Generators

| Generator | What it spawns | Notes |
|-----------|---------------|-------|
| **KnightGenerator** | Knights (and Golems at higher levels) | Spawn rate increases with level-timer progress; 3× slower during Ghost Mode |
| **PrincessGenerator** | Princesses | Fixed rate |
| **TraderGenerator** | Traders | Rate increases with level-timer progress |
| **MageGenerator** | Mages | Uses a faster spawn rate once Angry Mode is active |
| **SkellyGenerator** | Burst of 4–8 skeletons | Triggered by mage spell cast |
| **Sliminess** | Individual slimes | Timer-based; triggered by fireball hitting a slime |
| **MegaSliminess** | MegaSlimes | Timer-based |

---

## Special Game States

### Ghost Mode
Triggered when the level timer expires (full version only). The timer expiring is not a win — it just makes the game harder until the player fills the castles.
- Knights spawned from this point forward are Ghosts: same castle-pathfinding behavior, but 1.3× faster
- Knight spawn rate drops to 3× slower than normal
- On levels 7+, a Golem is also spawned when Ghost Mode starts
- Music switches to a slower theme

### Angry Mode
Triggered when a mage is killed (one-way flag; never resets within a level).
- The MageGenerator switches to its faster `dAngryRate` spawn rate, producing mages more frequently
- New mages spawned while Angry Mode is active are marked as "angry"
- Angry mages actively cast spells (every ~12 seconds on average): they stop moving, summon a burst of skeletons, then resume. They refuse to cast while close to a castle.
- Non-angry mages just walk their road harmlessly

---

## Levels

10 levels defined in `assets/levels.txt`. Each level specifies:
- Spawn frequencies for each enemy type
- Castle positions
- Road layout (paths princesses, traders, and mages travel)
- Time limit

**Progression:**
- Levels 1–3: Introduction — few enemies, 1–2 castles
- Levels 4–6: Mages and traders introduced, 2–3 castles, bonus durations shorter
- Levels 7–10: Multiple roads, 3–5 castles, mage skeleton counts increase, bonus durations shorter again

---

## Configuration (config.txt)

| Key | Options | Effect |
|-----|---------|--------|
| VERSION | `full` / `trial` | Unlocks all levels |
| PLATFORM | `pc` / `mobile` | Input/display mode |
| CONTROLS | `mouse` / `keyboard` / `joystick` | Changes initial fireball count (keyboard=6, mouse=4) |
| SCREEN_SIZE | `normal` / `small` | Display resolution |
| LOADING_SCREEN | `true` / `false` | Shows splash screen |
| DEBUG_UNIT_TEXT | `true` / `false` | Draws debug labels on entities |
