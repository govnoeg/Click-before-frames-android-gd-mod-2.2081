# v0.2.0
- Migrated the popup to the Geode v5 API.
- Corrected the Android Geometry Dash version metadata.
- Added the final approved mod icon to the project assets.
- Made the six tabs switch between separate pages.
- Added visible ON/OFF states and corrected the Wave Buffer range to 0–20F.
- Improved level profile keys and save/load feedback.
- Added Node IDs as an explicit dependency for the pause-menu button.
- Clarified that the gameplay input timing engine is not implemented yet.

# v0.1.0
- Initial Android-first UI and configuration scaffold.
- Pause menu entry point and initial Level Memory persistence.

## v0.3.0

- Implemented frame-step-delayed PlayerObject press/release dispatch.
- Paired press/release events so input hold duration is maintained.
- Added per-level click-location memory with automatic learning and persistent save.
- Level Memory bypasses normal Custom/Wave Buffer values and uses remembered location delays.
- Added learned-click count and clear-memory action to the Level Memory page.
- Clears pending inputs on pause, death, restart, quit, and completion.
- Changed Level Memory default to OFF so normal buffering is active by default.
