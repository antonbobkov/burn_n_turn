# prince_royals implementation summary

## Summary
The prince is in the game: each spawned royal is a 50/50 princess or prince, and towers show their first four royals in capture order.

- **Deviations:** small additions only. There's an extra test of the tower's draw calls, `Index::operator==` became `const`, and `TutorialTextEntity` got a getter for tests. The planned possible member for the figures wasn't needed.
- **Issues:** one test compile error (`Index::operator==` wasn't `const`), fixed in `code/utils/index.h`. A failed build also left old test binaries in place, and they still reported success.
- **Tips:**
    - Check the build's exit code before trusting test results, because tests run old binaries when a build fails.
    - Rename the type word by its uses, since `"princess"` also names art.
    - `royal_figures.txt` order must match the `RoyalKind` numbers.
    - A drawer that records calls tests drawing without needing real graphics.
- **Not verified:** nobody has looked at the result on a real screen yet. See step 9.

## Step 1: create this document
No deviations or issues.

## Step 2: rename to "royal"
- **Deviation:** code comments mentioning princesses all became "royals", beyond only the ones that named the renamed items. Both kinds are covered once the prince arrives.
- **Issues:** none. The build and all tests passed first time.
- **Tip:** the type word `"princess"` shares its spelling with art names such as `GetImgSeq("princess")`, so rename it by matching its uses (`GetType() ==`, `cCarry`), not the bare word.

## Step 3: tiny tower figures
- **Deviation:** none in the result. The script gained a BMP reader and writer for any size, because the existing ones only handle 16x16.
- **Issues:** none. Drawing the princess figure onto `castle0000.bmp` rebuilds `castle0001`-`0004.bmp` pixel for pixel.

## Step 4: royal kind and 50/50 spawning
No deviations or issues, apart from `MakeRoyal` taking the sequence by const reference to skip a copy.

## Step 5: dragon remembers who it carries
No deviations or issues.

## Step 6: tower stores and draws royals in order
- **Deviation:** the window positions are named constants instead of bare numbers. `GetImgSeq` returns a reference, so `Draw` makes no per-frame copy and no extra member was needed.
- **Issues:** none.

## Step 7: on-screen text
- **Deviation:** `README.md` also changed "damsels" to "royals", because the word no longer fits both.
- **Issues:** none. The longest new tutorial line is 52 characters. At 4 game pixels per character that's 208 pixels on a 480-pixel-wide screen (427 on the small screen setting), so it fits by calculation.

## Step 8: tests
- **Deviation:** added a test that records `Castle::Draw`'s calls and checks the base image, figure order, positions and the cap of four. Added `TutorialTextEntity::GetLatestText` so a test can read the tutorial's words.
- **Issue:** comparing `const Index` values failed to compile because `Index::operator==` wasn't `const`.
- **Solution:** made it `const`.
- **Issue:** that failed build still printed passing test results, from the old binaries.
- **Solution:** reran everything after a build with exit code 0.
- **Note:** the extra `rand()` call on each spawn didn't change the outcome of the existing scripted simulations, so they needed no timing changes.
- **Tip:** check the build's exit code before reading test results.

## Step 9: finalize
Debug and release builds pass with `-Werror`, integration and SDL tests pass, and `simulation_test` passes under AddressSanitizer and in release. The real game ran for several seconds under SDL's dummy video and audio drivers, loading the new files with no `error.txt`.

Still to do, by a person: play the game and look at it. Tests can't judge the look of mixed spawns, flipped walking, burning, or the tower figures on screen.
