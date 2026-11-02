#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include "CBFPopup.hpp"
#include "CBFConfig.hpp"
#include "CBFInputEngine.hpp"

using namespace geode::prelude;

class $modify(CBFPlayLayer, PlayLayer) {
    void setupHasCompleted() {
        PlayLayer::setupHasCompleted();
        cbf::input::clearPending();
        if (cbf::config::levelMemory()) {
            cbf::config::loadCurrentLevelProfile();
        }
        cbf::input::onLevelLoaded();
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        cbf::input::clearPending();
        PlayLayer::destroyPlayer(player, object);
    }

    void resetLevel() {
        cbf::input::clearPending();
        PlayLayer::resetLevel();
    }

    void pauseGame(bool paused) {
        if (paused) cbf::input::clearPending();
        PlayLayer::pauseGame(paused);
    }

    void resumeAndRestart(bool fromStart) {
        cbf::input::clearPending();
        PlayLayer::resumeAndRestart(fromStart);
    }

    void onQuit() {
        cbf::input::clearPending();
        PlayLayer::onQuit();
    }

    void levelComplete() {
        cbf::input::clearPending();
        PlayLayer::levelComplete();
    }
};

// Intercept the real player input calls. The engine only consumes an event
// when a delay is required; dispatching delayed events uses a re-entry guard
// so the original game method is called exactly once.
class $modify(CBFPlayerObject, PlayerObject) {
    bool pushButton(PlayerButton button) {
        if (cbf::input::intercept(this, button, true)) return true;
        return PlayerObject::pushButton(button);
    }

    bool releaseButton(PlayerButton button) {
        if (cbf::input::intercept(this, button, false)) return true;
        return PlayerObject::releaseButton(button);
    }
};

// processCommands is called by the game for its simulation steps. Counting
// these calls makes the configured delay independent of the display FPS.
class $modify(CBFBaseGameLayer, GJBaseGameLayer) {
    void processCommands(float dt, bool isHalfTick, bool isLastTick) {
        if (PlayLayer::get() == this) {
            cbf::input::advanceGameStep();
        }
        GJBaseGameLayer::processCommands(dt, isHalfTick, isLastTick);
    }
};

class $modify(CBFPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            return;
        }

        auto rightMenu = this->getChildByID("right-button-menu");
        if (!rightMenu) {
            log::warn("CBF: right-button-menu was not found; pause button was not added.");
            return;
        }

        auto resource = Mod::get()->getResourcesDir() / "cbf-button.png";
        auto sprite = CCSprite::create(resource.string().c_str());
        if (!sprite) {
            log::error("CBF: failed to load resources/cbf-button.png");
            return;
        }

        sprite->setScale(0.36f);
        auto button = CCMenuItemSpriteExtra::create(
            sprite, this, menu_selector(CBFPauseLayer::onOpenCBF)
        );
        if (!button) return;

        button->setID("cbf-open-button"_spr);
        rightMenu->addChild(button);
        rightMenu->updateLayout();
    }

    void onOpenCBF(CCObject*) {
        if (auto popup = CBFPopup::create()) {
            popup->show();
        }
    }
};
