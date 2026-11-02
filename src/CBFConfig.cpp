#include "CBFConfig.hpp"

#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <algorithm>
#include <cstdint>
#include <string_view>

using namespace geode::prelude;

namespace {
    template <class T>
    T read(char const* key, T fallback) {
        return Mod::get()->getSavedValue<T>(key, fallback);
    }

    template <class T>
    void write(char const* key, T value) {
        Mod::get()->setSavedValue<T>(key, value);
    }

    // Stable FNV-1a hash: avoids collisions caused by replacing every
    // punctuation character in a level name with the same underscore.
    std::uint64_t hashText(std::string_view text) {
        std::uint64_t hash = 14695981039346656037ull;
        for (unsigned char ch : text) {
            hash ^= ch;
            hash *= 1099511628211ull;
        }
        return hash;
    }

    void autoSaveProfile() {
        if (cbf::config::levelMemory() && cbf::config::robustSaves()) {
            cbf::config::saveCurrentLevelProfile();
        }
    }
}

namespace cbf::config {
    int bufferFrames() { return std::clamp(read<int>("buffer-frames", 10), 0, 30); }
    int waveBufferFrames() { return std::clamp(read<int>("wave-buffer-frames", 5), 0, 20); }
    int inputDelayFrames() { return std::clamp(read<int>("input-delay-frames", 0), 0, 20); }

    bool lowLatency() { return read<bool>("low-latency", true); }
    bool robustSaves() { return read<bool>("robust-saves", true); }
    bool orbAlignment() { return read<bool>("orb-alignment", true); }
    bool orbTiming() { return read<bool>("orb-timing", true); }
    bool shipBuffering() { return read<bool>("ship-buffering", false); }
    bool levelMemory() { return read<bool>("level-memory", false); }

    void setBufferFrames(int value) {
        write("buffer-frames", std::clamp(value, 0, 30));
        autoSaveProfile();
    }
    void setWaveBufferFrames(int value) {
        write("wave-buffer-frames", std::clamp(value, 0, 20));
        autoSaveProfile();
    }
    void setInputDelayFrames(int value) {
        write("input-delay-frames", std::clamp(value, 0, 20));
        autoSaveProfile();
    }

    void setLowLatency(bool value) { write("low-latency", value); autoSaveProfile(); }
    void setRobustSaves(bool value) {
        write("robust-saves", value);
        if (levelMemory()) saveCurrentLevelProfile();
    }
    void setOrbAlignment(bool value) { write("orb-alignment", value); autoSaveProfile(); }
    void setOrbTiming(bool value) { write("orb-timing", value); autoSaveProfile(); }
    void setShipBuffering(bool value) { write("ship-buffering", value); autoSaveProfile(); }
    void setLevelMemory(bool value) { write("level-memory", value); }

    std::string currentLevelKey() {
        auto playLayer = PlayLayer::get();
        if (!playLayer || !playLayer->m_level) {
            return {};
        }

        auto level = playLayer->m_level;
        auto levelID = level->m_levelID;
        auto levelType = static_cast<int>(level->m_levelType);

        // Uploaded/main levels have stable numeric IDs. Local/editor levels
        // can have ID 0, so use a stable hash of their name instead.
        if (levelID > 0) {
            return fmt::format("level-{}-{}", levelType, levelID);
        }

        auto name = std::string_view(level->m_levelName.c_str());
        return fmt::format("level-{}-local-{:016x}", levelType, hashText(name));
    }

    bool hasCurrentLevelProfile() {
        auto key = currentLevelKey();
        if (key.empty()) return false;
        return read<bool>((key + "-exists").c_str(), false);
    }

    bool saveCurrentLevelProfile() {
        if (!levelMemory()) return false;
        auto key = currentLevelKey();
        if (key.empty()) return false;

        write((key + "-buffer").c_str(), bufferFrames());
        write((key + "-wave").c_str(), waveBufferFrames());
        write((key + "-delay").c_str(), inputDelayFrames());
        write((key + "-low-latency").c_str(), lowLatency());
        write((key + "-robust-saves").c_str(), robustSaves());
        write((key + "-orb-alignment").c_str(), orbAlignment());
        write((key + "-orb-timing").c_str(), orbTiming());
        write((key + "-ship").c_str(), shipBuffering());
        write((key + "-exists").c_str(), true);
        return true;
    }

    bool loadCurrentLevelProfile() {
        if (!levelMemory()) return false;
        auto key = currentLevelKey();
        if (key.empty() || !read<bool>((key + "-exists").c_str(), false)) {
            return false;
        }

        // Read everything first, then update the active values. This avoids
        // intermediate autosaves changing values that have not been read yet.
        auto buffer = read<int>((key + "-buffer").c_str(), bufferFrames());
        auto wave = read<int>((key + "-wave").c_str(), waveBufferFrames());
        auto delay = read<int>((key + "-delay").c_str(), inputDelayFrames());
        auto latency = read<bool>((key + "-low-latency").c_str(), lowLatency());
        auto robust = read<bool>((key + "-robust-saves").c_str(), robustSaves());
        auto alignment = read<bool>((key + "-orb-alignment").c_str(), orbAlignment());
        auto timing = read<bool>((key + "-orb-timing").c_str(), orbTiming());
        auto ship = read<bool>((key + "-ship").c_str(), shipBuffering());

        // Temporarily suppress profile writes while restoring the profile.
        write("robust-saves", false);
        setBufferFrames(buffer);
        setWaveBufferFrames(wave);
        setInputDelayFrames(delay);
        setLowLatency(latency);
        setOrbAlignment(alignment);
        setOrbTiming(timing);
        setShipBuffering(ship);
        write("robust-saves", robust);
        return true;
    }
}
