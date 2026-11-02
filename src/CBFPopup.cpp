#include "CBFPopup.hpp"
#include "CBFConfig.hpp"
#include "CBFInputEngine.hpp"

#include <algorithm>

using namespace geode::prelude;

namespace {
    constexpr float kPopupWidth = 440.f;
    constexpr float kPopupHeight = 300.f;

    CCLabelBMFont* addText(CCNode* parent, char const* text, CCPoint pos, float scale = 0.38f) {
        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        if (!label) return nullptr;
        label->setScale(scale);
        label->setPosition(pos);
        parent->addChild(label);
        return label;
    }

    CCMenuItemLabel* addButton(CCMenu* menu, CCObject* target, SEL_MenuHandler selector,
                               char const* text, CCPoint pos, int tag = 0, float scale = 0.34f) {
        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        if (!label) return nullptr;
        label->setScale(scale);
        auto item = CCMenuItemLabel::create(label, target, selector);
        if (!item) return nullptr;
        item->setPosition(pos);
        item->setTag(tag);
        menu->addChild(item);
        return item;
    }

    CCMenu* makeMenu(CCNode* parent) {
        auto menu = CCMenu::create();
        menu->setPosition({ 0.f, 0.f });
        parent->addChild(menu);
        return menu;
    }

    void addCard(CCNode* parent, CCPoint center, CCSize size, ccColor3B tint) {
        auto card = CCScale9Sprite::create("square02_001.png");
        if (!card) return;
        card->setContentSize(size);
        card->setPosition(center);
        card->setColor(tint);
        card->setOpacity(205);
        parent->addChild(card, -1);

        auto accent = CCScale9Sprite::create("square02_001.png");
        if (!accent) return;
        accent->setContentSize({ 3.f, size.height - 10.f });
        accent->setPosition({ center.x - size.width / 2.f + 6.f, center.y });
        accent->setColor({ 35, 215, 255 });
        accent->setOpacity(235);
        parent->addChild(accent, -1);
    }
}

CBFPopup* CBFPopup::create() {
    auto ret = new CBFPopup();
    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool CBFPopup::init() {
    if (!Popup::init(kPopupWidth, kPopupHeight)) {
        return false;
    }
    buildUI();
    return true;
}

void CBFPopup::buildUI() {
    setTitle("CLICK BEFORE FRAMES", "bigFont.fnt", 0.58f);

    m_bgSprite->setColor({ 9, 12, 34 });
    m_bgSprite->setOpacity(245);

    auto panel = CCScale9Sprite::create("square02_001.png");
    panel->setContentSize({ 420.f, 190.f });
    panel->setPosition({ kPopupWidth / 2.f, 126.f });
    panel->setColor({ 15, 22, 55 });
    panel->setOpacity(238);
    m_mainLayer->addChild(panel, -1);

    addText(m_mainLayer, "MORE CONTROL. LESS LAG.", { kPopupWidth / 2.f, 267.f }, 0.27f);
    addText(m_mainLayer, "NETERMINDTEAM  |  ANDROID 64-BIT", { kPopupWidth / 2.f, 8.f }, 0.22f);

    auto tabs = makeMenu(m_mainLayer);
    const char* names[] = { "BUFFER", "MOVEMENT", "LEVEL", "PRACTICE", "DIAG", "SETTINGS" };
    const float positions[] = { 40.f, 112.f, 184.f, 256.f, 328.f, 400.f };
    for (int i = 0; i < 6; ++i) {
        m_tabButtons[i] = addButton(
            tabs, this, menu_selector(CBFPopup::onTab), names[i],
            { positions[i], 230.f }, i, 0.29f
        );
    }

    for (auto& page : m_pages) {
        page = CCNode::create();
        m_mainLayer->addChild(page);
    }

    buildBufferPage(m_pages[0]);
    buildMovementPage(m_pages[1]);
    buildLevelMemoryPage(m_pages[2]);
    buildPracticePage(m_pages[3]);
    buildDiagnosticsPage(m_pages[4]);
    buildSettingsPage(m_pages[5]);

    m_valueLabel = addText(m_mainLayer, "BUFFER 10F   WAVE 5F   DELAY 0F", { 174.f, 39.f }, 0.28f);
    m_statusLabel = addText(m_mainLayer, "ENGINE NOT ACTIVE  |  SETTINGS ONLY", { 220.f, 18.f }, 0.22f);

    auto footerMenu = makeMenu(m_mainLayer);
    addButton(footerMenu, this, menu_selector(CBFPopup::onTab), "MORE SETTINGS  >",
              { 354.f, 39.f }, 5, 0.24f);

    setTab(0);
    refresh();
}

void CBFPopup::buildBufferPage(CCNode* page) {
    for (float y : { 177.f, 143.f, 109.f, 75.f }) {
        addCard(page, { 109.f, y }, { 190.f, 29.f }, { 8, 17, 39 });
    }
    for (float y : { 177.f, 125.f, 73.f }) {
        addCard(page, { 318.f, y }, { 194.f, 43.f }, { 12, 14, 46 });
    }

    addText(page, "Low Latency", { 92.f, 177.f }, 0.34f);
    addText(page, "Robust Saves", { 92.f, 143.f }, 0.34f);
    addText(page, "Orb Alignment", { 92.f, 109.f }, 0.34f);
    addText(page, "Orb Timing", { 92.f, 75.f }, 0.34f);

    auto menu = makeMenu(page);
    m_lowLatencyButton = addButton(menu, this, menu_selector(CBFPopup::onToggleLowLatency), "ON", { 187.f, 177.f }, 0, 0.31f);
    m_robustSavesButton = addButton(menu, this, menu_selector(CBFPopup::onToggleRobustSaves), "ON", { 187.f, 143.f }, 0, 0.31f);
    m_orbAlignmentButton = addButton(menu, this, menu_selector(CBFPopup::onToggleOrbAlignment), "ON", { 187.f, 109.f }, 0, 0.31f);
    m_orbTimingButton = addButton(menu, this, menu_selector(CBFPopup::onToggleOrbTiming), "ON", { 187.f, 75.f }, 0, 0.31f);

    addText(page, "Input / Learn Delay", { 316.f, 177.f }, 0.30f);
    addText(page, "Custom Buffer", { 316.f, 125.f }, 0.30f);
    addText(page, "Wave Buffer", { 316.f, 73.f }, 0.30f);
    auto adjust = makeMenu(page);
    addButton(adjust, this, menu_selector(CBFPopup::onDelayDown), "-", { 250.f, 177.f }, 0, 0.40f);
    addButton(adjust, this, menu_selector(CBFPopup::onDelayUp), "+", { 382.f, 177.f }, 0, 0.40f);
    addButton(adjust, this, menu_selector(CBFPopup::onBufferDown), "-", { 250.f, 125.f }, 0, 0.40f);
    addButton(adjust, this, menu_selector(CBFPopup::onBufferUp), "+", { 382.f, 125.f }, 0, 0.40f);
    addButton(adjust, this, menu_selector(CBFPopup::onWaveDown), "-", { 250.f, 73.f }, 0, 0.40f);
    addButton(adjust, this, menu_selector(CBFPopup::onWaveUp), "+", { 382.f, 73.f }, 0, 0.40f);
}

void CBFPopup::buildMovementPage(CCNode* page) {
    addCard(page, { 220.f, 130.f }, { 370.f, 52.f }, { 10, 18, 46 });
    addText(page, "MOVEMENT OPTIONS", { 220.f, 177.f }, 0.42f);
    addText(page, "Ship Buffering", { 115.f, 130.f }, 0.36f);
    addText(page, "Wave Buffer uses the value from BUFFER.", { 220.f, 82.f }, 0.27f);
    auto menu = makeMenu(page);
    m_shipBufferButton = addButton(menu, this, menu_selector(CBFPopup::onToggleShip), "OFF", { 300.f, 130.f }, 0, 0.34f);
}

void CBFPopup::buildLevelMemoryPage(CCNode* page) {
    addCard(page, { 220.f, 112.f }, { 370.f, 146.f }, { 14, 17, 52 });
    addText(page, "LEVEL MEMORY", { 220.f, 180.f }, 0.42f);
    addText(page, "Learns a delay for each click location on this level.", { 220.f, 151.f }, 0.255f);
    addText(page, "Normal Custom/Wave Buffer is bypassed in this mode.", { 220.f, 131.f }, 0.235f);
    auto menu = makeMenu(page);
    m_levelMemoryButton = addButton(menu, this, menu_selector(CBFPopup::onToggleLevelMemory), "OFF", { 220.f, 101.f }, 0, 0.34f);
    m_memoryCountLabel = addText(page, "LEARNED CLICK SPOTS: 0", { 220.f, 82.f }, 0.28f);
    addText(page, "New spots use the Input / Learn Delay from BUFFER.", { 220.f, 60.f }, 0.225f);
    addButton(menu, this, menu_selector(CBFPopup::onClearClickMemory), "CLEAR THIS LEVEL'S CLICK MEMORY", { 220.f, 39.f }, 0, 0.245f);
}

void CBFPopup::buildPracticePage(CCNode* page) {
    addText(page, "PRACTICE", { 220.f, 166.f }, 0.46f);
    addText(page, "Practice-mode automation is not connected yet.", { 220.f, 118.f }, 0.29f);
    addText(page, "This page is reserved for future practice tools.", { 220.f, 82.f }, 0.27f);
}

void CBFPopup::buildDiagnosticsPage(CCNode* page) {
    addText(page, "DIAGNOSTICS", { 220.f, 166.f }, 0.46f);
    addText(page, "UI / configuration: READY", { 220.f, 124.f }, 0.31f);
    addText(page, "Input hooks implemented — test in game", { 220.f, 90.f }, 0.29f);
    addText(page, "Delay is counted in game simulation steps.", { 220.f, 58.f }, 0.25f);
}

void CBFPopup::buildSettingsPage(CCNode* page) {
    addText(page, "GENERAL SETTINGS", { 220.f, 166.f }, 0.44f);
    addText(page, "Robust Saves", { 115.f, 120.f }, 0.35f);
    addText(page, "Level Memory", { 115.f, 80.f }, 0.35f);
    auto menu = makeMenu(page);
    addButton(menu, this, menu_selector(CBFPopup::onToggleRobustSaves), "TOGGLE", { 310.f, 120.f }, 0, 0.31f);
    addButton(menu, this, menu_selector(CBFPopup::onToggleLevelMemory), "TOGGLE", { 310.f, 80.f }, 0, 0.31f);
}

void CBFPopup::refresh() {
    if (m_valueLabel) {
        auto values = fmt::format("BUFFER {}F   WAVE {}F   DELAY {}F",
            cbf::config::bufferFrames(), cbf::config::waveBufferFrames(), cbf::config::inputDelayFrames());
        m_valueLabel->setString(values.c_str());
    }

    auto updateToggle = [](CCMenuItemLabel* button, bool enabled) {
        if (button) {
            button->setString(enabled ? "ON" : "OFF");
            button->setColor(enabled ? cc3(30, 225, 255) : cc3(180, 180, 205));
        }
    };
    updateToggle(m_lowLatencyButton, cbf::config::lowLatency());
    updateToggle(m_robustSavesButton, cbf::config::robustSaves());
    updateToggle(m_orbAlignmentButton, cbf::config::orbAlignment());
    updateToggle(m_orbTimingButton, cbf::config::orbTiming());
    updateToggle(m_shipBufferButton, cbf::config::shipBuffering());
    updateToggle(m_levelMemoryButton, cbf::config::levelMemory());
    if (m_memoryCountLabel) {
        auto count = fmt::format("LEARNED CLICK SPOTS: {}", cbf::input::rememberedPointCount());
        m_memoryCountLabel->setString(count.c_str());
    }
}

void CBFPopup::setStatus(char const* text) {
    if (m_statusLabel) m_statusLabel->setString(text);
}

void CBFPopup::setTab(int tab) {
    m_tab = std::clamp(tab, 0, 5);
    for (int i = 0; i < 6; ++i) {
        if (m_pages[i]) m_pages[i]->setVisible(i == m_tab);
        if (m_tabButtons[i]) {
            m_tabButtons[i]->setColor(i == m_tab ? cc3(35, 220, 255) : cc3(185, 175, 220));
        }
    }

}

void CBFPopup::onTab(CCObject* sender) {
    if (!sender) return;
    setTab(static_cast<CCNode*>(sender)->getTag());
}

void CBFPopup::onBufferDown(CCObject*) { cbf::config::setBufferFrames(cbf::config::bufferFrames() - 1); refresh(); }
void CBFPopup::onBufferUp(CCObject*) { cbf::config::setBufferFrames(cbf::config::bufferFrames() + 1); refresh(); }
void CBFPopup::onWaveDown(CCObject*) { cbf::config::setWaveBufferFrames(cbf::config::waveBufferFrames() - 1); refresh(); }
void CBFPopup::onWaveUp(CCObject*) { cbf::config::setWaveBufferFrames(cbf::config::waveBufferFrames() + 1); refresh(); }
void CBFPopup::onDelayDown(CCObject*) { cbf::config::setInputDelayFrames(cbf::config::inputDelayFrames() - 1); refresh(); }
void CBFPopup::onDelayUp(CCObject*) { cbf::config::setInputDelayFrames(cbf::config::inputDelayFrames() + 1); refresh(); }

void CBFPopup::onToggleLowLatency(CCObject*) { cbf::config::setLowLatency(!cbf::config::lowLatency()); refresh(); }
void CBFPopup::onToggleRobustSaves(CCObject*) { cbf::config::setRobustSaves(!cbf::config::robustSaves()); refresh(); }
void CBFPopup::onToggleOrbAlignment(CCObject*) { cbf::config::setOrbAlignment(!cbf::config::orbAlignment()); refresh(); }
void CBFPopup::onToggleOrbTiming(CCObject*) { cbf::config::setOrbTiming(!cbf::config::orbTiming()); refresh(); }
void CBFPopup::onToggleShip(CCObject*) { cbf::config::setShipBuffering(!cbf::config::shipBuffering()); refresh(); }
void CBFPopup::onToggleLevelMemory(CCObject*) {
    cbf::config::setLevelMemory(!cbf::config::levelMemory());
    cbf::input::clearPending();
    refresh();
    setStatus(cbf::config::levelMemory() ? "LEVEL MEMORY ON: NORMAL BUFFER BYPASSED" : "NORMAL CLICK BUFFER RESTORED");
}

void CBFPopup::onSaveLevel(CCObject*) {
    setStatus(cbf::config::saveCurrentLevelProfile() ? "CURRENT LEVEL PROFILE SAVED" : "CANNOT SAVE: LOAD A LEVEL AND ENABLE MEMORY");
}

void CBFPopup::onLoadLevel(CCObject*) {
    auto loaded = cbf::config::loadCurrentLevelProfile();
    refresh();
    setStatus(loaded ? "CURRENT LEVEL PROFILE LOADED" : "NO PROFILE FOUND FOR THIS LEVEL");
}

void CBFPopup::onClearClickMemory(CCObject*) {
    cbf::input::clearCurrentLevelMemory();
    refresh();
    setStatus("CLICK MEMORY CLEARED FOR THIS LEVEL");
}
