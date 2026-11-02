# Click Before Frames

Android-first Geode source project for Geometry Dash **2.2081**.

- Geode SDK target: **5.9.0**
- Platform: **Android64**
- Mod ID: `netermindteam.clickbeforeframes`
- Project version: **v0.3.0**
- Developer: **NetermindTeam**

## What is included

- Approved mod icon, pause-menu icon, and supplied menu-style reference.
- Six popup pages: Buffer, Movement, Level Memory, Practice, Diagnostics, and Settings.
- **Real input delay:** hooks `PlayerObject::pushButton` and `releaseButton`; press/release are queued and delivered after the selected number of game simulation steps.
- Main mode uses Custom Buffer as the base delay, Wave Buffer while the player is in Wave, and adds Input Delay as a global offset. Ship input uses the main buffer only when Ship Buffering is enabled. The result is clamped to 0–30 steps.
- **Level Memory mode:** bypasses the normal Custom/Wave Buffer values. At a new player position it learns the configured Input Delay; later clicks at the same level location (within a small position tolerance) use that remembered delay.
- Each memory point is scoped to the current level key and stored persistently. The popup shows the learned point count and can clear the current level's click memory.
- Press and release use the same delay so the length of a held input is preserved. Delayed inputs are cleared on pause, death, restart, quit, and level completion to prevent stale input after state changes.
- Per-level settings profiles from v0.2.0 remain available.

## Input timing behaviour

### Normal Click Buffering

`effective delay = (Wave Buffer if in Wave, otherwise Custom Buffer) + Input Delay`

### Level Memory

When enabled, the normal buffer is bypassed. For every new click location the mod saves the current Input Delay as that location's learned delay. On later attempts, the mod matches the player's current position and input button to the nearest learned point and applies its remembered delay. The mod never creates clicks on its own: it only delays player-generated press/release events.

Use the Input Delay value as the learning delay for spots the mod has not seen before. A value of 0F is remembered as an immediate click. The lookup separates player 1/player 2, input button, and Wave vs non-Wave mode. To relearn a level from scratch, clear that level's click memory. Local levels with ID 0 use a stable name hash as their key, so two local levels with identical names may share memory.

## Verification boundary

The source has static integrity checks, but it has **not** been compiled against the user's installed Geode SDK or tested in Geometry Dash in this environment. The Low Latency, Orb Alignment, and Orb Timing toggles remain UI/configuration placeholders in this milestone. The exact runtime timing, frame-step cadence, Android input routing, and interaction with other input-hook mods must be verified in-game. A successful GitHub Actions build is required before treating the binary as build-verified.

## Build on GitHub

1. Upload the contents of this folder to the root of the `ClickBeforeFrames` repository.
2. Keep `mod.json`, `CMakeLists.txt`, `src/`, `resources/`, and `.github/` at the repository root.
3. Open the **Actions** tab and run **Build Click Before Frames (Android64)**, or push a commit to `main`.
4. Download the `ClickBeforeFrames-android64` artifact from the successful workflow run.

## Local build

With the Geode 5.9.0 SDK/CLI and Android64 toolchain installed:

```bash
geode build -p android64 --config Release -- -G Ninja
```

The package is expected in `build-android64/`.

## Verification boundary

This source bundle has been checked for archive integrity, JSON validity, required project files, and basic source consistency. This environment does not have the Geode SDK or Android NDK installed, so an actual Android compilation and in-game test have **not** been performed here. GitHub Actions must pass before treating the `.geode` package as build-verified.
