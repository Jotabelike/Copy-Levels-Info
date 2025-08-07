#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Mod.hpp>
#include <sstream>

using namespace geode::prelude;

std::string getLevelDifficulty(GJGameLevel* level) {
    if (!level) return "Unknown";

    if (level->m_autoLevel) return "Auto";
    if (level->m_demon) {
        switch (level->m_demonDifficulty) {
        case 3: return "Easy Demon";
        case 4: return "Medium Demon";
        case 5: return "Hard Demon";
        case 6: return "Insane Demon";
        case 7: return "Extreme Demon";
        default: return "Demon";
        }
    }

    switch (static_cast<GJDifficulty>(level->m_difficulty)) {
    case GJDifficulty::NA: return "NA";
    case GJDifficulty::Easy: return "Easy";
    case GJDifficulty::Normal: return "Normal";
    case GJDifficulty::Hard: return "Hard";
    case GJDifficulty::Harder: return "Harder";
    case GJDifficulty::Insane: return "Insane";
    case GJDifficulty::Demon: return "Demon";
    default: return "Unknown";
    }
}

void copyLevelInfo(GJGameLevel* level) {
    if (!level) {
        Notification::create("Error: invalid level", NotificationIcon::Error)->show();
        return;
    }

    auto mod = Mod::get();

    bool copyLevelID = mod->getSettingValue<bool>("copy-level-id");
    bool copyLevelName = mod->getSettingValue<bool>("copy-level-name");
    bool copySongID = mod->getSettingValue<bool>("copy-song-id");
    bool copyCreator = mod->getSettingValue<bool>("copy-creator");
    bool copyStars = mod->getSettingValue<bool>("copy-stars");
    bool copyDifficulty = mod->getSettingValue<bool>("copy-difficulty");

    std::stringstream info;
    if (copyLevelID)
        info << "ID: " << level->m_levelID.value() << "\n";
    if (copyLevelName)
        info << "Level Name: " << level->m_levelName.c_str() << "\n";
    if (copySongID)
        info << "Song ID: " << level->m_songID << "\n";
    if (copyCreator)
        info << "Creator: " << level->m_creatorName.c_str() << "\n";
    if (copyStars)
        info << "Stars: " << level->m_stars << "\n";
    if (copyDifficulty)
        info << "Dificulty: " << getLevelDifficulty(level) << "\n";

    if (geode::utils::clipboard::write(info.str())) {
        Notification::create("Copied!", NotificationIcon::Success)->show();
    }
    else {
        Notification::create("Error", NotificationIcon::Error)->show();
    }
}

class $modify(LevelInfoLayerCopyHook, LevelInfoLayer) {
public:
    struct Fields {
        CCMenu* m_copyMenu = nullptr;
        CCMenuItemSpriteExtra* m_copyBtn = nullptr;
    };

    bool init(GJGameLevel * level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        auto label = CCLabelBMFont::create("COPY", "bigFont.fnt");
        label->setScale(0.7f);
        auto copySpr = CircleButtonSprite::create(label, CircleBaseColor::Pink, CircleBaseSize::Medium);
        copySpr->setScale(0.65f);

        m_fields->m_copyBtn = CCMenuItemSpriteExtra::create(
            copySpr,
            this,
            menu_selector(LevelInfoLayerCopyHook::onCopy)
        );

        m_fields->m_copyMenu = CCMenu::create();
        m_fields->m_copyMenu->addChild(m_fields->m_copyBtn);

        if (auto likeBtn = this->getChildByID("like-button")) {
            m_fields->m_copyMenu->setPosition(
                likeBtn->getPositionX() - 70.f,
                likeBtn->getPositionY()
            );
        }
        else {
            m_fields->m_copyMenu->setPosition(106.f, 30.f);
        }

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(m_fields->m_copyMenu);
        }
        else {
            this->addChild(m_fields->m_copyMenu);
        }

        return true;
    }

    void onCopy(CCObject*) {
        copyLevelInfo(m_level);
    }
};