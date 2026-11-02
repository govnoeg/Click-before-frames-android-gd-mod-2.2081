#include "CBFInputEngine.hpp"
#include "CBFConfig.hpp"

#include <Geode/binding/PlayLayer.hpp>
#include <Geode/binding/GJGameLevel.hpp>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace cbf::input {
namespace {
    constexpr int kMaximumDelay = 30;
    constexpr std::size_t kMaximumMemoryPoints = 2048;
    constexpr int kPositionCellSize = 8;
    constexpr int kMatchRadiusX = 3; // 24 game units
    constexpr int kMatchRadiusY = 6; // 48 game units

    struct MemoryPoint {
        int x = 0;
        int y = 0;
        int button = 0;
        int player = 1;
        int mode = 0; // 1 = Wave, 0 = other modes
        int delay = 0;
    };

    struct PendingInput {
        PlayerObject* player = nullptr;
        PlayerButton button = PlayerButton::Jump;
        bool isPress = false;
        std::uint64_t dueStep = 0;
        std::uint64_t sequence = 0;
    };

    struct ChannelState {
        int heldDelay = 0;
        std::uint64_t lastDueStep = 0;
        bool hasHeldPress = false;
    };

    std::vector<PendingInput> g_pending;
    std::unordered_map<std::uint64_t, ChannelState> g_channels;
    std::vector<MemoryPoint> g_memoryPoints;
    std::string g_memoryLevelKey;
    std::uint64_t g_gameStep = 0;
    std::uint64_t g_sequence = 0;
    bool g_dispatching = false;

    std::string memoryStorageKey(std::string const& levelKey) {
        return levelKey + "-click-memory-v1";
    }

    std::uint64_t channelKey(PlayerObject* player, PlayerButton button) {
        auto address = reinterpret_cast<std::uintptr_t>(player);
        auto buttonValue = static_cast<std::uint64_t>(static_cast<int>(button));
        return (static_cast<std::uint64_t>(address) >> 4) ^ (buttonValue << 56);
    }

    int playerNumber(PlayerObject* player) {
        auto layer = PlayLayer::get();
        if (layer && layer->m_player2 == player) return 2;
        return 1;
    }

    int quantize(float coordinate) {
        return static_cast<int>(std::lround(coordinate / static_cast<float>(kPositionCellSize)));
    }

    void saveMemoryPoints() {
        if (g_memoryLevelKey.empty()) return;

        std::ostringstream out;
        for (auto const& point : g_memoryPoints) {
            out << point.x << '|'
                << point.y << '|'
                << point.button << '|'
                << point.player << '|'
                << point.mode << '|'
                << point.delay << '\n';
        }
        Mod::get()->setSavedValue<std::string>(memoryStorageKey(g_memoryLevelKey), out.str());
    }

    void loadMemoryPoints(std::string const& levelKey) {
        g_memoryPoints.clear();
        g_memoryLevelKey = levelKey;
        if (levelKey.empty()) return;

        auto data = Mod::get()->getSavedValue<std::string>(memoryStorageKey(levelKey), "");
        std::istringstream input(data);
        std::string line;
        while (std::getline(input, line) && g_memoryPoints.size() < kMaximumMemoryPoints) {
            std::istringstream row(line);
            MemoryPoint point;
            char separator1 = 0, separator2 = 0, separator3 = 0, separator4 = 0, separator5 = 0;
            if (!(row >> point.x >> separator1 >> point.y >> separator2
                      >> point.button >> separator3 >> point.player >> separator4 >> point.mode >> separator5 >> point.delay)) {
                continue;
            }
            if (separator1 != '|' || separator2 != '|' || separator3 != '|' || separator4 != '|' || separator5 != '|') {
                continue;
            }
            if (point.player < 1 || point.player > 2 || point.mode < 0 || point.mode > 1 || point.delay < 0 || point.delay > kMaximumDelay) {
                continue;
            }
            g_memoryPoints.push_back(point);
        }
    }

    void ensureMemoryForCurrentLevel() {
        auto key = cbf::config::currentLevelKey();
        if (key != g_memoryLevelKey) loadMemoryPoints(key);
    }

    MemoryPoint* findMemoryPoint(int x, int y, int button, int player, int mode) {
        MemoryPoint* best = nullptr;
        int bestDistance = std::numeric_limits<int>::max();

        for (auto& point : g_memoryPoints) {
            if (point.button != button || point.player != player || point.mode != mode) continue;
            int dx = std::abs(point.x - x);
            int dy = std::abs(point.y - y);
            if (dx > kMatchRadiusX || dy > kMatchRadiusY) continue;
            int distance = dx * dx + dy * dy;
            if (distance < bestDistance) {
                bestDistance = distance;
                best = &point;
            }
        }
        return best;
    }

    int delayForPress(PlayerObject* player, PlayerButton button) {
        if (cbf::config::levelMemory()) {
            ensureMemoryForCurrentLevel();
            auto layer = PlayLayer::get();
            if (!layer || g_memoryLevelKey.empty()) return cbf::config::inputDelayFrames();

            int x = quantize(player->getPositionX());
            int y = quantize(player->getPositionY());
            int buttonValue = static_cast<int>(button);
            int playerValue = playerNumber(player);
            int modeValue = player->m_isDart ? 1 : 0;

            if (auto point = findMemoryPoint(x, y, buttonValue, playerValue, modeValue)) {
                // The remembered delay replaces the normal buffer entirely.
                return std::clamp(point->delay, 0, kMaximumDelay);
            }

            // First visit to this spot: learn the user's selected fallback delay.
            int learnedDelay = std::clamp(cbf::config::inputDelayFrames(), 0, kMaximumDelay);
            if (g_memoryPoints.size() < kMaximumMemoryPoints) {
                g_memoryPoints.push_back({ x, y, buttonValue, playerValue, modeValue, learnedDelay });
                saveMemoryPoints();
            }
            return learnedDelay;
        }

        // Normal mode: Custom Buffer is the main delay. Wave Buffer replaces it
        // while the player is in Wave. Input Delay is an extra frame offset.
        int baseDelay = 0;
        if (player->m_isDart) {
            baseDelay = cbf::config::waveBufferFrames();
        } else if (!player->m_isShip || cbf::config::shipBuffering()) {
            baseDelay = cbf::config::bufferFrames();
        }
        return std::clamp(baseDelay + cbf::config::inputDelayFrames(), 0, kMaximumDelay);
    }
}

bool intercept(PlayerObject* player, PlayerButton button, bool isPress) {
    if (g_dispatching || !player) return false;

    auto layer = PlayLayer::get();
    if (!layer || layer->m_playerDied || (player != layer->m_player1 && player != layer->m_player2)) {
        return false;
    }

    auto key = channelKey(player, button);
    auto& channel = g_channels[key];

    int delay = 0;
    if (isPress) {
        delay = delayForPress(player, button);
        channel.heldDelay = delay;
        channel.hasHeldPress = true;
    } else {
        // Keep press/release paired: the release receives the same delay as its press.
        delay = channel.hasHeldPress ? channel.heldDelay : 0;
        channel.hasHeldPress = false;
    }

    delay = std::clamp(delay, 0, kMaximumDelay);
    if (delay <= 0) {
        if (!isPress) g_channels.erase(key);
        return false;
    }

    std::uint64_t due = g_gameStep + static_cast<std::uint64_t>(delay);
    // A release must never be delivered before its corresponding press.
    if (!isPress) due = std::max(due, channel.lastDueStep);
    channel.lastDueStep = due;

    g_pending.push_back({ player, button, isPress, due, ++g_sequence });
    if (!isPress) {
        // Preserve lastDueStep until the paired event is dispatched.
        channel.hasHeldPress = false;
    }
    return true;
}

void advanceGameStep() {
    auto layer = PlayLayer::get();
    if (!layer || layer->m_playerDied) {
        clearPending();
        return;
    }

    ++g_gameStep;

    std::vector<PendingInput> ready;
    auto it = g_pending.begin();
    while (it != g_pending.end()) {
        if (it->dueStep <= g_gameStep) {
            ready.push_back(*it);
            it = g_pending.erase(it);
        } else {
            ++it;
        }
    }

    if (ready.empty()) return;
    std::stable_sort(ready.begin(), ready.end(), [](PendingInput const& a, PendingInput const& b) {
        if (a.dueStep != b.dueStep) return a.dueStep < b.dueStep;
        return a.sequence < b.sequence;
    });

    g_dispatching = true;
    for (auto const& event : ready) {
        if (!event.player) continue;
        auto currentLayer = PlayLayer::get();
        if (!currentLayer || currentLayer->m_playerDied) break;
        if (event.player != currentLayer->m_player1 && event.player != currentLayer->m_player2) continue;

        if (event.isPress) event.player->pushButton(event.button);
        else event.player->releaseButton(event.button);
    }
    g_dispatching = false;

    // Remove channel records that no longer have delayed events waiting.
    for (auto channel = g_channels.begin(); channel != g_channels.end();) {
        bool hasPending = std::any_of(g_pending.begin(), g_pending.end(), [&](PendingInput const& event) {
            return channel->first == channelKey(event.player, event.button);
        });
        if (!hasPending && !channel->second.hasHeldPress) channel = g_channels.erase(channel);
        else ++channel;
    }
}

void clearPending() {
    g_pending.clear();
    g_channels.clear();
    g_gameStep = 0;
    g_dispatching = false;
}

void onLevelLoaded() {
    clearPending();
    loadMemoryPoints(cbf::config::currentLevelKey());
}

int rememberedPointCount() {
    ensureMemoryForCurrentLevel();
    return static_cast<int>(g_memoryPoints.size());
}

void clearCurrentLevelMemory() {
    ensureMemoryForCurrentLevel();
    g_memoryPoints.clear();
    saveMemoryPoints();
}
}
