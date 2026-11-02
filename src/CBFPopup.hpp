#pragma once

#include <Geode/Geode.hpp>
#include <array>

class CBFPopup : public geode::Popup {
protected:
    int m_tab = 0;
    std::array<cocos2d::CCNode*, 6> m_pages{};
    std::array<cocos2d::CCMenuItemLabel*, 6> m_tabButtons{};

    cocos2d::CCLabelBMFont* m_valueLabel = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;
    cocos2d::CCLabelBMFont* m_memoryCountLabel = nullptr;

    cocos2d::CCMenuItemLabel* m_lowLatencyButton = nullptr;
    cocos2d::CCMenuItemLabel* m_robustSavesButton = nullptr;
    cocos2d::CCMenuItemLabel* m_orbAlignmentButton = nullptr;
    cocos2d::CCMenuItemLabel* m_orbTimingButton = nullptr;
    cocos2d::CCMenuItemLabel* m_shipBufferButton = nullptr;
    cocos2d::CCMenuItemLabel* m_levelMemoryButton = nullptr;

    bool init();
    void buildUI();
    void buildBufferPage(cocos2d::CCNode* page);
    void buildMovementPage(cocos2d::CCNode* page);
    void buildLevelMemoryPage(cocos2d::CCNode* page);
    void buildPracticePage(cocos2d::CCNode* page);
    void buildDiagnosticsPage(cocos2d::CCNode* page);
    void buildSettingsPage(cocos2d::CCNode* page);
    void refresh();
    void setTab(int tab);
    void setStatus(char const* text);

    void onTab(cocos2d::CCObject* sender);
    void onBufferDown(cocos2d::CCObject*);
    void onBufferUp(cocos2d::CCObject*);
    void onWaveDown(cocos2d::CCObject*);
    void onWaveUp(cocos2d::CCObject*);
    void onDelayDown(cocos2d::CCObject*);
    void onDelayUp(cocos2d::CCObject*);
    void onToggleLowLatency(cocos2d::CCObject*);
    void onToggleRobustSaves(cocos2d::CCObject*);
    void onToggleOrbAlignment(cocos2d::CCObject*);
    void onToggleOrbTiming(cocos2d::CCObject*);
    void onToggleShip(cocos2d::CCObject*);
    void onToggleLevelMemory(cocos2d::CCObject*);
    void onSaveLevel(cocos2d::CCObject*);
    void onLoadLevel(cocos2d::CCObject*);
    void onClearClickMemory(cocos2d::CCObject*);

public:
    static CBFPopup* create();
};
