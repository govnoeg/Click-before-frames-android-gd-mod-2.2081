# Click Before Frames — v0.3.0 source audit

## v0.2.0 work retained

- Geode v5 Popup migration and Android 2.2081 metadata.
- Approved avatar and pause-menu button artwork.
- Six independent popup pages and the More Settings navigation.
- Per-level settings profiles keyed by level type + numeric ID, with a local-level name-hash fallback.
- Android64 GitHub Actions workflow pinned to Geode SDK 5.9.0.

## v0.3.0 gameplay input work

- Added a `PlayerObject::pushButton` / `releaseButton` input interception layer.
- Delayed press and release events are queued separately but paired with the same delay so tap/hold duration is retained.
- The queue advances from `GJBaseGameLayer::processCommands`, so the counter follows game simulation steps rather than display refresh callbacks.
- Normal mode uses Custom Buffer (or Wave Buffer while `m_isDart` is true) plus Input Delay, capped at 30 steps. Ship uses the main buffer only when Ship Buffering is enabled.
- Level Memory bypasses Custom/Wave Buffer. It learns the Input Delay value at each new player location and reuses it on later attempts near that location. Records are separated by level key, player index, button, and Wave/non-Wave mode.
- Level click memory is persisted in a compact per-level string value, limited to 2,048 points per level.
- Pending events are cleared on pause, death, restart, quit, completion, and level setup.
- Level Memory defaults to OFF so normal Click Buffering is active on a fresh install.

## Static checks performed

- `mod.json` parses and targets Geometry Dash Android 2.2081 / Geode 5.9.0.
- CMake includes the new input engine source.
- The source contains matching input hooks, simulation-step advancement, paired release dispatch, level-scoped memory serialization, and clear-memory UI.
- All six UI pages and image assets remain present.
- `tests/check-project.py` passes.

## Not verified here

- No Geode SDK / Android NDK C++ build was run in this environment.
- No in-game Android test was run. The hook and timing logic must still be checked against the exact installed SDK build and other mods that also intercept player input.
- The Level Memory's first-visit delay comes from the Input / Learn Delay setting. Low Latency, Orb Alignment, and Orb Timing are still UI/configuration placeholders. It is not a physics predictor and does not invent or replay clicks; it only delays clicks that the player actually makes.
- Position matching uses a tolerance of 24 world units horizontally and 48 vertically. Different routes or unusual player positions may need tuning after on-device testing.
- Local levels with ID 0 use a level-name hash, so local levels with identical names may share stored data.

A successful GitHub Actions build is still required for compile verification. An in-game test is required for runtime verification.
