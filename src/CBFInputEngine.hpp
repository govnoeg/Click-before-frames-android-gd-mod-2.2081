#pragma once

#include <Geode/Geode.hpp>
#include <Geode/binding/PlayerObject.hpp>

namespace cbf::input {
    // Returns true when the input is consumed and scheduled for a later game step.
    // Returns false when the original PlayerObject method should run immediately.
    bool intercept(PlayerObject* player, PlayerButton button, bool isPress);

    // Called once from GJBaseGameLayer::processCommands for the active PlayLayer.
    void advanceGameStep();

    // Clear delayed inputs on pause, death, restart, quit, or level transition.
    void clearPending();

    // Load this level's learned click-delay map.
    void onLevelLoaded();

    // Level Memory helpers used by the popup.
    int rememberedPointCount();
    void clearCurrentLevelMemory();
}
