#pragma once

#include <Geode/Geode.hpp>
#include <string>

namespace cbf::config {
    int bufferFrames();
    int waveBufferFrames();
    int inputDelayFrames();

    bool lowLatency();
    bool robustSaves();
    bool orbAlignment();
    bool orbTiming();
    bool shipBuffering();
    bool levelMemory();

    void setBufferFrames(int value);
    void setWaveBufferFrames(int value);
    void setInputDelayFrames(int value);

    void setLowLatency(bool value);
    void setRobustSaves(bool value);
    void setOrbAlignment(bool value);
    void setOrbTiming(bool value);
    void setShipBuffering(bool value);
    void setLevelMemory(bool value);

    std::string currentLevelKey();
    bool hasCurrentLevelProfile();
    bool saveCurrentLevelProfile();
    bool loadCurrentLevelProfile();
}
