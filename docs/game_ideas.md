# Game Ideas — New Behaviors

Brainstormed extensions to existing entities. None of these are implemented.

---

## Princess Variants & Behaviors

### Frightened Princess
The princess mistakes the dragon for a monster. When the dragon flies within ~80px, she runs in the opposite direction along her road. The player must either cut her off (approach from the other direction) or wait for her to calm down after a few seconds once the dragon backs off. Makes early rescues trickier.

### Brave Princess
Instead of just walking her road helplessly, this princess is self-aware. She slowly walks toward the nearest castle on her own (same pathfinding as a knight, but half speed). The dragon doesn't need to carry her — just escort her safely. Skeletons still kill her. Rewards players who protect her rather than babysit.

### Cursed Princess
A princess that has been touched by a mage's aura. She wanders randomly like a slime instead of following her road. Visual cue: purple tint. The dragon must still pick her up and deliver her, but finding her is harder.

### Princess That Signals
A princess that occasionally stops and waves her arms (animation only), producing a visible pulsing ring visible across the map. Helps the player locate her but also draws nearby skeletons toward the signal.

---

## Knight Variants

### Dodging Knight (Strong Knight)
When a fireball is fired directly at this knight, it has a ~40% chance to sidestep perpendicular to the fireball's path. Only dodges if the fireball is within a certain range and heading straight at it. Looks like it sees the fireball coming and jumps aside. Requires the player to lead shots or wait for it to commit to a direction. Still 1 hit to kill once struck.

### Shield Knight
Carries a shield on its front-facing side. Fireballs that hit it from the front are deflected (reflected back, or just disappear). Fireballs from the side or behind kill it normally. Forces the player to flank or use the Explode/Chain bonus to hit it from an angle.

### Archer Knight
Stops periodically when it has line-of-sight to the dragon and fires an arrow. The arrow travels in a straight line. If it hits the dragon, one fireball charge is knocked loose and dropped as a pickup on the ground. Doesn't destroy the dragon — just annoys and depletes ammo. Keeps moving toward the castle between shots.

### Berserker Knight
Starts at normal speed. Each time a fireball hits it and it survives (requires it to have some HP, e.g. 3 hits), it speeds up. At 1 HP remaining it moves at ~1.5× base speed. Creates tension: chip damage makes it more dangerous.

### Knight Carrying a Princess
A knight that captured a princess in the field. It walks toward the castle carrying her. If you kill the knight, the princess is freed (dropped as a live entity). If the knight reaches the castle, the princess is lost (locked away, not rescuable). Visual: princess sprite is shown above the knight sprite.

### Commander Knight
A rare knight wearing a different color. Nearby knights (within ~100px) move 25% faster while the commander is alive. Killing the commander debuffs the group back to normal speed. Incentivizes prioritizing the commander.

---

## Skeleton Variants

### Armored Skeleton
Takes 2 hits to kill. First hit breaks its armor (visual change). Still kills princesses and traders on contact.

### Leaping Skeleton
Instead of walking in a straight line, it occasionally leaps forward 50–80px in its movement direction. Harder to hit with slow fireballs. Same 1 hit to kill.

---

## Trader Variants & Behaviors

### Haggling Trader
Won't hand over his bonus the moment he's delivered. Instead, after the dragon drops him at a castle, a short timer (~4 seconds) counts down before the fireball upgrade is granted. During that window the player just waits — creates a brief pause of tension rather than an instant reward.

### Wounded Trader
Moves at half speed and has a visible limp animation. Easy target for skeletons, but drops a better bonus than the normal trader (e.g. a guaranteed Explode or Laser rather than a random pick). Worth going out of your way to protect.

### Cowardly Trader
When a skeleton or knight comes within ~90px, he panics and runs off his road in a random direction. The player must herd him back (fly near him) to calm him down. If he runs off-screen he's lost.

### Merchant Caravan
Two traders that walk the road together. Delivering both to the same castle in one trip (carry one, escort the other at the same time) grants a bonus upgrade of the next tier — essentially a buy-one-get-one. Delivering just one is still the normal reward. Skeletons kill either of them individually.

### Trader That Bribes Knights
When a knight walks near this trader (within ~50px), the trader throws coins at it. The knight stops for ~3 seconds to pick them up before resuming its march. Passive ability — the player doesn't control it. Buying a few seconds of distraction.

---

## Mage Variants

### Dark Mage
A more powerful mage variant that appears at higher levels. Instead of summoning skeletons, it summons a burst of armored skeletons. Also has a second spell: it temporarily curses the nearest princess, turning her Cursed (see above).

### Mage That Resurrects Knights
When a knight is killed near this mage (within ~100px), the mage raises it as a skeleton. One resurrection per killed knight. Encourages killing the mage first.

### Blinking Mage
When a fireball is on a direct collision course within ~70px, the mage teleports to a random nearby position along its road. Can only blink once every ~5 seconds. Forces the player to anticipate and fire ahead, or wait out the cooldown.

### Shielding Mage
Periodically (every ~15 seconds) casts a protective ward on a random nearby knight, giving it a visible glowing shield that blocks the next fireball. The ward breaks on first hit. The shielded knight still dies on the second hit. Priority target to kill before it can keep warding.

### Slowing Mage
Instead of summoning skeletons, its angry-mode spell fires a wave that hits all fireballs on screen, reducing their speed by half for ~5 seconds. Doesn't kill fireballs — just makes them crawl. Layering this with a knight rush creates a nasty combo.

### Buffing Mage
Its presence (within ~120px) passively speeds up all knights in range by 20%. Doesn't cast spells itself. Killing it immediately removes the speed aura. The player learns to spot it visually (different color/hat) and prioritize it over the surrounding knights.

---

## Misc / Systemic Ideas

### Knight Rally Point
When 3+ knights are within ~60px of each other, they bunch up and move as a group, slightly faster than individuals. Visually looks like a squad. Splitting them up (e.g. with Explode) breaks the bonus.

### Princess Rescued by Trader
If a trader walks close enough to a princess on the same road, the trader picks her up and escorts her toward the nearest castle autonomously. The dragon doesn't need to intervene — but the trader is now a high-priority skeleton target since killing him loses both the trader bonus and the princess.

### Exhausted Dragon Mechanic
After carrying a passenger (princess or trader) for more than ~10 seconds without landing, the dragon's movement speed slowly decreases. Encourages players to quickly deliver rather than flying around with a passenger indefinitely.
