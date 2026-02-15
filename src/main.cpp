#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/Loader.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace geode::prelude;

 

class ExposeScroll : public ScrollLayer {
public:
    using ScrollLayer::ccTouchBegan;
    using ScrollLayer::ccTouchMoved;
    using ScrollLayer::ccTouchEnded;
    using ScrollLayer::ccTouchCancelled;
};

 

static std::string getLevelDifficulty(GJGameLevel* level) {
    if (!level) return "Unknown";
    if (level->m_demon) {
        switch (level->m_demonDifficulty) {
        case 3: return "Easy Demon";
        case 4: return "Medium Demon";
        case 5: return "Insane Demon";
        case 6: return "Extreme Demon";
        default: return "Hard Demon";
        }
    }
    if (level->m_autoLevel || level->m_stars == 1) return "Auto";
    if (level->m_stars == 2) return "Easy";
    if (level->m_stars == 3) return "Normal";
    if (level->m_stars == 4 || level->m_stars == 5) return "Hard";
    if (level->m_stars == 6 || level->m_stars == 7) return "Harder";
    if (level->m_stars == 8 || level->m_stars == 9) return "Insane";
    return (level->m_stars == 0) ? "Unrated" : "Unknown";
}

static std::string getLengthName(int levelLength) {
    switch (levelLength) {
    case 0: return "Tiny";
    case 1: return "Short";
    case 2: return "Medium";
    case 3: return "Long";
    case 4: return "XL";
    default: return "Unknown";
    }
}

static std::string getSongInfo(GJGameLevel* level) {
    if (level->m_songID == 0) return "Official (" + std::to_string(level->m_audioTrack) + ")";
    return std::to_string(level->m_songID);
}

static std::string getGameMode(GJGameLevel* level) {
    if (level->isPlatformer()) return "Platformer";
    return "Classic";
}

 

enum class CopyType {
    ID, Name, Song, Creator, Stars, Difficulty, Objects, Length, GameMode
};

struct CopyDef {
    CopyType type;
    std::string defaultName;
    std::string saveKey;
    std::function<std::string(GJGameLevel*)> getValue;
};

 

class CopyRow : public CCNode {
public:
    CopyDef m_def;
    CCMenuItemToggler* m_toggle = nullptr;
    CCSprite* m_dragHandle = nullptr;
    CCMenu* m_menu = nullptr;
    CCScale9Sprite* m_bg = nullptr;

    float m_width = 300.f;
    float m_height = 35.f;

    static CopyRow* create(CopyDef def, float width, GJGameLevel* level) {
        auto ret = new CopyRow();
        if (ret && ret->init(def, width, level)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool init(CopyDef def, float width, GJGameLevel* level) {
        if (!CCNode::init()) return false;
        m_def = def;
        m_width = width;
        this->setContentSize(CCSize(width, m_height));
        this->setAnchorPoint(CCPoint(0.5f, 0.5f));
 
        m_bg = CCScale9Sprite::create("square02_small.png");
        m_bg->setContentSize(CCSize(width, m_height));
        m_bg->setOpacity(50);  
        m_bg->setPosition(CCPoint(width / 2, m_height / 2));
        this->addChild(m_bg);

        m_menu = CCMenu::create();
        m_menu->setPosition(CCPoint(0, 0));
        m_menu->setContentSize(CCSize(width, m_height));
        this->addChild(m_menu);

         
        bool isToggled = Mod::get()->getSavedValue<bool>(def.saveKey, true);
        m_toggle = CCMenuItemToggler::createWithStandardSprites(this, nullptr, 0.6f);
        m_toggle->setPosition(CCPoint(20.f, m_height / 2));
        m_toggle->toggle(isToggled);
        m_menu->addChild(m_toggle);

       
        std::string valText = def.getValue(level);
        if (valText.empty()) valText = "-";

        auto valLabel = CCLabelBMFont::create(valText.c_str(), "bigFont.fnt");
        valLabel->setAnchorPoint(CCPoint(0.f, 0.5f));
        valLabel->setPosition(CCPoint(45.f, 22.f));

        float maxWidth = width - 80.f;
        if (valLabel->getContentSize().width > maxWidth) {
            valLabel->setScale(0.5f * (maxWidth / valLabel->getContentSize().width));
        }
        else {
            valLabel->setScale(0.5f);
        }
        this->addChild(valLabel);

        
        auto nameLabel = CCLabelBMFont::create(def.defaultName.c_str(), "goldFont.fnt");
        nameLabel->setScale(0.35f);
        nameLabel->setAnchorPoint(CCPoint(0.f, 0.5f));
        nameLabel->setPosition(CCPoint(45.f, 10.f));
        this->addChild(nameLabel);

        
        m_dragHandle = CCSprite::createWithSpriteFrameName("edit_flipXBtn_001.png");
        m_dragHandle->setScale(0.6f);
        m_dragHandle->setRotation(90.0f);
        m_dragHandle->setPosition(CCPoint(width - 20.f, m_height / 2));
        m_dragHandle->setColor({ 180, 180, 180 });
        this->addChild(m_dragHandle);

        return true;
    }

    void onEnter() override {
        CCNode::onEnter();
        if (m_menu) {
            Loader::get()->queueInMainThread([this] {
                if (this->getParent() && m_menu) {
                    m_menu->setHandlerPriority(-505);
                }
                });
        }
    }

    void savePreference() {
        Mod::get()->setSavedValue(m_def.saveKey, m_toggle->isToggled());
    }

    void setDraggingState(bool dragging) {
        this->stopAllActions();
        if (dragging) {
            this->setZOrder(10);
            this->setScale(1.02f);
            m_bg->setColor({ 100, 255, 100 });
            m_bg->setOpacity(150);
        }
        else {
            this->setZOrder(0);
            this->setScale(1.0f);
            m_bg->setColor({ 255, 255, 255 });
            m_bg->setOpacity(50);
        }
    }
};

 

class CopyInfoPopup : public geode::Popup {
protected:
    GJGameLevel* m_level;
    std::vector<CopyRow*> m_rows;
    ScrollLayer* m_scrollLayer = nullptr;
    CCNode* m_content = nullptr;
    CopyRow* m_draggingRow = nullptr;
    CCMenuItemToggler* m_toggleLabels;

    float m_dragStartY = 0.f;
    float m_rowOriginalY = 0.f;
    float m_rowHeight = 40.f;

    enum TouchMode { IDLE, SCROLLING, DRAGGING };
    TouchMode m_touchMode = IDLE;

    std::vector<CopyDef> getAllDefs() {
        return {
            {CopyType::ID, "Level ID", "chk-id", [](GJGameLevel* l) { return std::to_string(l->m_levelID.value()); }},
            {CopyType::Name, "Level Name", "chk-name", [](GJGameLevel* l) { return l->m_levelName; }},
            {CopyType::Song, "Song ID", "chk-song", [](GJGameLevel* l) { return getSongInfo(l); }},
            {CopyType::Creator, "Creator", "chk-creator", [](GJGameLevel* l) { return l->m_creatorName; }},
            {CopyType::Stars, "Stars", "chk-stars", [](GJGameLevel* l) { return std::to_string(l->m_stars); }},
            {CopyType::Difficulty, "Difficulty", "chk-diff", [](GJGameLevel* l) { return getLevelDifficulty(l); }},
            {CopyType::Objects, "Objects", "chk-objs", [](GJGameLevel* l) { return std::to_string(l->m_objectCount); }},
            {CopyType::Length, "Duration", "chk-dur", [](GJGameLevel* l) { return getLengthName(l->m_levelLength); }},
            {CopyType::GameMode, "Game Mode", "chk-mode", [](GJGameLevel* l) { return getGameMode(l); }}
        };
    }

    bool init(GJGameLevel* level) {
        float popupW = 360.f;
        float popupH = 260.f;

         
        if (!Popup::init(popupW, popupH)) return false;

        m_level = level;
        this->setTitle("Copy Info");

       
        CCTouchDispatcher::get()->addTargetedDelegate(static_cast<CCTouchDelegate*>(this), -500, true);
 
        auto resetSpr = CCSprite::createWithSpriteFrameName("GJ_resetBtn_001.png");
        resetSpr->setScale(0.8f);
        auto resetBtn = CCMenuItemSpriteExtra::create(
            resetSpr, this, menu_selector(CopyInfoPopup::onReset)
        );
        auto resetMenu = CCMenu::create();
        resetMenu->addChild(resetBtn);
        resetMenu->setPosition(CCPoint(popupW - 25.f, popupH - 25.f));
        this->m_mainLayer->addChild(resetMenu);
 
        float scrollW = 310.f;
        float scrollH = 140.f;

        
        auto listBg = CCScale9Sprite::create("square02_small.png");
        listBg->setContentSize(CCSize(scrollW, scrollH));
        listBg->setOpacity(100);
        listBg->setPosition(CCPoint(popupW / 2, 145.f));
        this->m_mainLayer->addChild(listBg);

        m_scrollLayer = ScrollLayer::create(CCSize(scrollW, scrollH));
        m_scrollLayer->setPosition(CCPoint((popupW - scrollW) / 2, 75.f));
        m_scrollLayer->setTouchEnabled(false);
        CCTouchDispatcher::get()->removeDelegate(m_scrollLayer);

        this->m_mainLayer->addChild(m_scrollLayer);
        m_content = m_scrollLayer->m_contentLayer;

        reloadRows();
        m_scrollLayer->scrollToTop();

      
        auto lblMenu = CCMenu::create();
        lblMenu->setPosition(CCPoint(40.f, 35.f));
        this->m_mainLayer->addChild(lblMenu);

        bool lblToggled = Mod::get()->getSavedValue<bool>("chk-labels", true);
        m_toggleLabels = CCMenuItemToggler::createWithStandardSprites(this, nullptr, 0.6f);
        m_toggleLabels->toggle(lblToggled);
        m_toggleLabels->setPosition(CCPoint(0, 0));
        lblMenu->addChild(m_toggleLabels);

        auto lblText = CCLabelBMFont::create("Labels", "bigFont.fnt");
        lblText->setScale(0.4f);
        lblText->setPosition(CCPoint(20.f, 0));
        lblText->setAnchorPoint(CCPoint(0, 0.5f));
        lblMenu->addChild(lblText);

    
        auto btnSprite = CCSprite::createWithSpriteFrameName("GJ_copyBtn_001.png");
        btnSprite->setScale(0.85f);

        auto copyBtn = CCMenuItemSpriteExtra::create(
            btnSprite,
            this,
            menu_selector(CopyInfoPopup::onCopyAndClose)
        );

        auto btnMenu = CCMenu::create();
        btnMenu->setPosition(CCPoint(popupW - 40.f, 35.f));
        btnMenu->addChild(copyBtn);
        this->m_mainLayer->addChild(btnMenu);

        return true;
    }

    void reloadRows() {
        m_rows.clear();
        m_content->removeAllChildren();

        auto allDefs = getAllDefs();
        auto savedOrder = Mod::get()->getSavedValue<std::vector<int>>("sort-order-v2");
        std::vector<CopyDef> sortedDefs;

        if (savedOrder.empty() || savedOrder.size() != allDefs.size()) {
            sortedDefs = allDefs;
        }
        else {
            for (int typeInt : savedOrder) {
                for (const auto& def : allDefs) {
                    if ((int)def.type == typeInt) {
                        sortedDefs.push_back(def);
                        break;
                    }
                }
            }
            if (sortedDefs.size() != allDefs.size()) sortedDefs = allDefs;
            if (sortedDefs.size() < allDefs.size()) {
                for (auto& def : allDefs) {
                    bool exists = false;
                    for (auto& s : sortedDefs) if (s.type == def.type) exists = true;
                    if (!exists) sortedDefs.push_back(def);
                }
            }
        }

        float rowWidth = 300.f;

        for (const auto& def : sortedDefs) {
            auto row = CopyRow::create(def, rowWidth, m_level);
            m_rows.push_back(row);
            m_content->addChild(row);
        }

        float totalHeight = std::max(140.f, m_rows.size() * m_rowHeight + 10.f);
        m_content->setContentSize(CCSize(310.f, totalHeight));
        updateRowPositions(false);
    }

    void updateRowPositions(bool animate) {
        float totalHeight = m_content->getContentSize().height;
        float startY = totalHeight - (m_rowHeight / 2) - 5.f;

        for (int i = 0; i < m_rows.size(); i++) {
            auto row = m_rows[i];
            if (row == m_draggingRow) continue;

            float targetY = startY - (i * m_rowHeight);
            CCPoint targetPos = CCPoint(155.f, targetY); 

            if (animate) {
                if (row->getPosition() != targetPos) {
                    row->stopAllActions();
                    auto move = CCMoveTo::create(0.2f, targetPos);
                    auto ease = CCEaseOut::create(move, 2.0f);
                    row->runAction(ease);
                }
            }
            else {
                row->setPosition(targetPos);
            }
        }
    }

    void onReset(CCObject*) {
        geode::createQuickPopup(
            "Reset Defaults",
            "Are you sure you want to reset all copy settings?",
            "Cancel", "Reset",
            [this](auto, bool btn2) {
                if (btn2) {
                    Mod::get()->setSavedValue("sort-order-v2", std::vector<int>());
                    Mod::get()->setSavedValue("chk-labels", true);

                    auto allDefs = getAllDefs();
                    for (const auto& def : allDefs) {
                        Mod::get()->setSavedValue(def.saveKey, true);
                    }

                    this->onClose(nullptr);
                    CopyInfoPopup::create(m_level)->show();
                }
            }
        );
    }

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        auto location = touch->getLocation();

        for (auto row : m_rows) {
            CCPoint handleWorldPos = row->m_dragHandle->getParent()->convertToWorldSpace(row->m_dragHandle->getPosition());

            if (ccpDistance(handleWorldPos, location) < 25.f) {
                m_draggingRow = row;
                m_dragStartY = location.y;
                m_rowOriginalY = row->getPositionY();
                m_touchMode = DRAGGING;
                row->setDraggingState(true);
                return true;
            }
        }

        CCPoint scrollLocal = m_scrollLayer->convertToNodeSpace(location);
        CCRect scrollRect = CCRect(0, 0, m_scrollLayer->getContentSize().width,
        m_scrollLayer->getContentSize().height);

        if (scrollRect.containsPoint(scrollLocal)) {
            m_touchMode = SCROLLING;
            static_cast<ExposeScroll*>(m_scrollLayer)->ccTouchBegan(touch, event);
            return true;
        }

        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (m_touchMode == DRAGGING && m_draggingRow) {
            float diff = touch->getLocation().y - m_dragStartY;
            float newY = m_rowOriginalY + diff;
            float totalH = m_content->getContentSize().height;
            if (newY > totalH - 20) newY = totalH - 20;
            if (newY < 20) newY = 20;

            m_draggingRow->setPositionY(newY);

            float startY = totalH - (m_rowHeight / 2) - 5.f;
            int potentialIndex = std::round((startY - newY) / m_rowHeight);

            if (potentialIndex < 0) potentialIndex = 0;
            if (potentialIndex >= m_rows.size()) potentialIndex = m_rows.size() - 1;

            auto it = std::find(m_rows.begin(), m_rows.end(), m_draggingRow);
            int currentIndex = std::distance(m_rows.begin(), it);

            if (potentialIndex != currentIndex) {
                m_rows.erase(it);
                m_rows.insert(m_rows.begin() + potentialIndex, m_draggingRow);
                updateRowPositions(true);
            }
        }
        else if (m_touchMode == SCROLLING) {
            static_cast<ExposeScroll*>(m_scrollLayer)->ccTouchMoved(touch, event);
        }
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        if (m_touchMode == DRAGGING && m_draggingRow) {
            m_draggingRow->setDraggingState(false);
            m_draggingRow = nullptr;
            updateRowPositions(true);
        }
        else if (m_touchMode == SCROLLING) {
            static_cast<ExposeScroll*>(m_scrollLayer)->ccTouchEnded(touch, event);
        }
        m_touchMode = IDLE;
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        if (m_touchMode == DRAGGING && m_draggingRow) {
            m_draggingRow->setDraggingState(false);
            m_draggingRow = nullptr;
            updateRowPositions(true);
        }
        else if (m_touchMode == SCROLLING) {
            static_cast<ExposeScroll*>(m_scrollLayer)->ccTouchCancelled(touch, event);
        }
        m_touchMode = IDLE;
    }

    void onCopyAndClose(CCObject*) {
        std::stringstream info;
        std::vector<int> saveOrder;

        bool includeLabels = m_toggleLabels->isToggled();
        Mod::get()->setSavedValue("chk-labels", includeLabels);

        for (auto row : m_rows) {
            row->savePreference();
            saveOrder.push_back((int)row->m_def.type);

            if (row->m_toggle->isToggled()) {
                std::string val = row->m_def.getValue(m_level);
                if (includeLabels) {
                    info << row->m_def.defaultName << ": " << val << "\n";
                }
                else {
                    info << val << "\n";
                }
            }
        }

        Mod::get()->setSavedValue("sort-order-v2", saveOrder);

        std::string out = info.str();
        if (!out.empty() && out.back() == '\n') out.pop_back();

        if (geode::utils::clipboard::write(out)) {
            Notification::create("Copied!", NotificationIcon::Success)->show();
        }
        else {
            Notification::create("Error", NotificationIcon::Error)->show();
        }

        CCTouchDispatcher::get()->removeDelegate(static_cast<CCTouchDelegate*>(this));
        if (m_scrollLayer) CCTouchDispatcher::get()->removeDelegate(m_scrollLayer);

        this->onClose(nullptr);
    }

    void onClose(CCObject* obj) override {
        CCTouchDispatcher::get()->removeDelegate(static_cast<CCTouchDelegate*>(this));
        if (m_scrollLayer) CCTouchDispatcher::get()->removeDelegate(m_scrollLayer);
        Popup::onClose(obj);
    }

public:
    static CopyInfoPopup* create(GJGameLevel* level) {
        auto ret = new CopyInfoPopup();
        if (ret && ret->init(level)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

 

class $modify(LevelInfoLayerCopyHook, LevelInfoLayer) {
    struct Fields {
        CCMenu* menu = nullptr;
    };

    bool init(GJGameLevel * level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;
 
        auto spr = CCSprite::createWithSpriteFrameName("GJ_copyBtn_001.png");
        spr->setScale(0.7f);

        auto btn = CCMenuItemSpriteExtra::create(
            spr,
            this,
            menu_selector(LevelInfoLayerCopyHook::onOpenCopyPopup)
        );

        m_fields->menu = CCMenu::create();
        m_fields->menu->addChild(btn);

      
        if (auto likeBtn = this->getChildByID("like-button")) {
            m_fields->menu->setPosition(CCPoint(
                likeBtn->getPositionX() - 50.f,
                likeBtn->getPositionY()
            ));
        }
        else {
            m_fields->menu->setPosition(CCPoint(106.f, 30.f));
        }

        if (auto bottom = this->getChildByID("bottom-menu")) {
            bottom->addChild(m_fields->menu);
        }
        else {
            this->addChild(m_fields->menu);
        }

        return true;
    }

    void onOpenCopyPopup(CCObject*) {
        CopyInfoPopup::create(m_level)->show();
    }
};